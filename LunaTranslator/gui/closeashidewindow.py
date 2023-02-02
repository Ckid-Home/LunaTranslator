
from PyQt5.QtWidgets import  QMainWindow 
from PyQt5.QtGui import QFont,QCloseEvent
from PyQt5.QtCore import Qt,pyqtSignal ,QSize,QPoint
class closeashidewindow(QMainWindow): 
    showsignal=pyqtSignal()
    def __init__(self, args) -> None:
        super().__init__(args )
        self.showsignal.connect(self.showfunction)  
    def showfunction(self): 
        if self.isMinimized():
            self.showNormal() 
        elif self.isHidden(): 
            self.show()  
        else:
            self.hide()  
     
    def closeEvent(self, event:QCloseEvent) :  
        self.hide() 
        event.ignore() 