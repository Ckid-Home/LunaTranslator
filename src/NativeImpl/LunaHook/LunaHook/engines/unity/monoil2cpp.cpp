#include "def_mono.hpp"
#include "def_il2cpp.hpp"
extern CommonSharedMem *commonsharedmem;
namespace
{
    std::optional<std::wstring_view> readmonostring(void *ptr)
    {
        if (!ptr)
            return {};
        MonoString *string = (MonoString *)ptr;
        auto data = (wchar_t *)string->chars;
        auto len = string->length;
        if (!(len && data))
            return {};
        if (wcslen(data) != len)
            return {};
        return std::wstring_view(data, len);
    }
    void *createmonostring(std::wstring_view ws, MonoString *origin)
    {
        auto newstring = (MonoString *)malloc(sizeof(MonoString) + ws.size() + 2);
        MonoString __;
        origin = origin ? origin : &__;
        memcpy(newstring, origin, sizeof(MonoString));
        memcpy((wchar_t *)newstring->chars, ws.data(), ws.size() * 2);
        newstring->length = ws.size();
        return newstring;
    }
}
std::optional<std::wstring_view> commonsolvemonostring(uintptr_t arg)
{
    auto sw = il2cppfunctions::get_string((void *)arg);
    if (!sw)
        sw = monofunctions::get_string((void *)arg);
    if (!sw)
        sw = readmonostring((void *)arg);
    if (!sw)
        return {};
    if (sw.value().size() > TEXT_BUFFER_SIZE)
        return {};
    return sw;
}
void *create_string_csharp(std::wstring_view view, void *origin)
{
    auto newstring = il2cppfunctions::create_string(view);
    if (!newstring)
        newstring = monofunctions::create_string(view);
    if (!newstring)
        newstring = createmonostring(view, (MonoString *)origin);
    return newstring;
}
void unity_ui_string_embed_fun(uintptr_t &arg, TextBuffer buff)
{
    auto view = buff.viewW();
    arg = (uintptr_t)create_string_csharp(view, (void *)arg);
}

uintptr_t tryfindmonoil2cpp(const char *_dll, const char *_namespace, const char *_class, const char *_method, int paramCoun, bool strict)
{
    auto addr = il2cppfunctions::get_method_pointer(_dll, _namespace, _class, _method, paramCoun, strict);
    if (addr)
        return addr;
    return monofunctions::get_method_pointer(_dll, _namespace, _class, _method, paramCoun, strict);
}
void *tryfindmonoil2cppMethod(const char *_dll, const char *_namespace, const char *_class, const char *_method, int paramCoun, bool strict)
{
    auto addr = il2cppfunctions::get_method_internal(_dll, _namespace, _class, _method, paramCoun, strict);
    if (addr)
        return (void *)addr;
    return (void *)monofunctions::get_method_internal(_dll, _namespace, _class, _method, paramCoun, strict);
}
void *tryfindmonoil2cppType(const char *_dll, const char *_namespace, const char *_class, bool strict)
{
    auto addr = il2cppfunctions::get_type_pointer(_dll, _namespace, _class, strict);
    if (addr)
        return (void *)addr;
    return (void *)monofunctions::get_type_pointer(_dll, _namespace, _class, strict);
}
void *tryfindmonoil2cppClass(const char *_dll, const char *_namespace, const char *_class, bool strict)
{
    auto addr = il2cppfunctions::get_class_pointer(_dll, _namespace, _class, strict);
    if (addr)
        return (void *)addr;
    return (void *)monofunctions::get_class_pointer(_dll, _namespace, _class, strict);
}
std::variant<monoloopinfo, il2cpploopinfo> loop_all_methods(std::optional<std::function<void(const std::string &)>> show)
{
    auto ms = il2cppfunctions::loop_all_methods(show);
    if (ms.size())
        return ms;
    return monofunctions::loop_all_methods(show);
}

namespace monoil2cpp
{
    static std::atomic<int> g_font_state{0};

    static void *g_applyFont_m = nullptr;

    struct thread_scope
    {
        void *thread = nullptr;
        thread_scope();
        ~thread_scope();
    };

    bool is_il2cpp() { return il2cpp_runtime_invoke != nullptr; }

    thread_scope::thread_scope()
    {
        if (is_il2cpp())
        {
            auto d = (SafeFptr(il2cpp_domain_get))();
            if (d)
                thread = (SafeFptr(il2cpp_thread_attach))(d);
        }
        else
        {
            auto d = (SafeFptr(mono_get_root_domain))();
            if (d)
                thread = (SafeFptr(mono_thread_attach))(d);
        }
    }
    thread_scope::~thread_scope() {}

    void *invoke(void *method, void *obj, void **params, void **exc_out)
    {
        if (!method)
            return nullptr;
        thread_scope ts;
        void *exc = nullptr;
        void *ret = nullptr;
        if (is_il2cpp())
            ret = (SafeFptr(il2cpp_runtime_invoke))((MethodInfo *)method, obj, params, (Il2CppObject **)&exc);
        else
            ret = (SafeFptr(mono_runtime_invoke))((MonoMethod *)method, obj, params, (MonoObject **)&exc);
        if (exc_out)
            *exc_out = exc;
        return ret;
    }

    void log_managed_exception(void *exc)
    {
        if (!exc)
            return;
        const char *name = nullptr;
        if (is_il2cpp())
        {
            auto klass = ((Il2CppObject *)exc)->klass;
            if (klass)
                name = (SafeFptr(il2cpp_class_get_name))(klass);
        }
        else
        {
            auto klass = (SafeFptr(mono_object_get_class))((MonoObject *)exc);
            if (klass)
                name = (SafeFptr(mono_class_get_name))(klass);
        }
        if (name)
            Msg::Log(name);
        void *toStringM = tryfindmonoil2cppMethod("mscorlib", "System", "Exception", "ToString", 0);
        if (!toStringM)
            return;
        void *ex2 = nullptr;
        void *ret = invoke(toStringM, exc, nullptr, &ex2);
        if (!ret)
            return;
        if (auto sw = commonsolvemonostring((uintptr_t)ret))
        {
            Msg::Log(WideStringToString(sw.value()).c_str());
        }
    }

    static std::filesystem::path resolve_plugin_path()
    {
        const wchar_t *name = L"LunaTmpFontLoader.dll";
        wchar_t dllpath[MAX_PATH];
        GetModuleFileNameW(GetModuleHandle(LUNA_HOOK_DLL), dllpath, MAX_PATH);
        return (std::filesystem::path(dllpath).parent_path() / name);
    }

    static void load_managed_plugin_impl()
    {
        thread_scope ts;
        if (is_il2cpp())
        {
            g_font_state = -1;
            return;
        }
        int status = 0;
        auto assembly = (SafeFptr(mono_assembly_open))(resolve_plugin_path().string().c_str(), &status);
        if (!assembly)
        {
            g_font_state = -1;
            return;
        }
        auto image = (SafeFptr(mono_assembly_get_image))(assembly);
        if (!image)
        {
            g_font_state = -1;
            return;
        }
        auto klass = (SafeFptr(mono_class_from_name))(image, "LunaTmpFontLoader", "FontLoader");
        if (!klass)
        {
            g_font_state = -1;
            return;
        }
        auto method = (SafeFptr(mono_class_get_method_from_name))(klass, "LoadFont", 1);
        if (!method)
        {
            g_font_state = -1;
            return;
        }
        g_applyFont_m = (void *)(SafeFptr(mono_class_get_method_from_name))(klass, "ApplyFont", 1);
        auto bundleDirStr = create_string_csharp(commonsharedmem->unityfontdir);
        void *args[1] = {bundleDirStr};
        void *exc = nullptr;
        invoke((void *)method, nullptr, args, &exc);
        if (exc)
        {
            log_managed_exception(exc);
            g_font_state = -1;
            return;
        }
        g_font_state = 4;
    }

    void apply_font(void *self)
    {
        if (!self || !g_applyFont_m)
            return;
        void *args[1] = {self};
        void *exc = nullptr;
        invoke(g_applyFont_m, nullptr, args, &exc);
        if (exc)
            log_managed_exception(exc);
    }

    static void load_on_main_seh()
    {
        __try
        {
            load_managed_plugin_impl();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            g_font_state = -1;
        }
    }

    void ensure_tmp_font_loaded()
    {
        if (!commonsharedmem || !commonsharedmem->unityfontdir[0])
            return;
        int s = g_font_state.load(std::memory_order_relaxed);
        if (s == 4 || s == -1)
            return;
        int expected = 0;
        if (!g_font_state.compare_exchange_strong(expected, 1, std::memory_order_acq_rel))
            return;
        load_on_main_seh();
    }
}