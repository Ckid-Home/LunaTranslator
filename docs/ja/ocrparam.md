# OCR自動化実行方法

さまざまなゲームの異なるテキスト更新方式に対応するため、ソフトウェアには、一定の頻度でゲームから自動的にスクリーンショットを撮影する4つの異なる方法があります。

## 定期実行

::: tip
他の設定のパラメータがわかりにくい場合は、この方法を使用してください。この方法が最も簡単な方法です。

:::

この方法は、「実行間隔」に基づいて定期的に実行され。


## 画像更新の分析

この方法は、「画像安定性しきい値」「画像一貫性しきい値」などのパラメータを使用します。

1. #### 画像安定性しきい値

    ゲームのテキストがすぐに表示されない場合（テキスト速度が最速でない場合）や、ゲームに動的な背景やlive2D要素がある場合、キャプチャされた画像は継続的に変化します。

    スクリーンショットを撮るたびに、前のスクリーンショットと比較して類似度を計算します。類似度がしきい値を超えると、画像が安定していると見なされ、次のステップが実行されます。

    ゲームが完全に静的であることが確認できる場合、この値を0に設定できます。逆に、そうでない場合は、この値を適切に増やす必要があります。

1. #### 画像一貫性しきい値

    このパラメータは最も重要です。

    画像が安定した後、現在の画像を前回のOCR実行時の画像（最後のスクリーンショットではない）と比較します。類似度がこのしきい値を下回ると、ゲームテキストが変更されたと見なされ、OCRが実行されます。

    OCRの頻度が高すぎる場合、この値を適切に増やすことができます。逆に、遅すぎる場合は、この値を適切に減らすことができます。

## マウス/キーボードトリガー + 安定待ち

1. #### トリガーイベント

    デフォルトでは、次のマウス/キーボードイベントがこの方法をトリガーします：左マウスボタンの押下、Enterキーの押下、Ctrlキーの解放、Shiftキーの解放、Altキーの解放。ゲームウィンドウがバインドされている場合、この方法はゲームウィンドウがフォアグラウンドウィンドウである場合にのみトリガーされます。

    この方法をトリガーした後、ゲームが新しいテキストをレンダリングするために短い待機期間が必要です。テキストがすぐに表示されない場合（テキスト速度が最速でない場合）を考慮します。

    この方法がトリガーされ、安定性が達成されると、テキストの類似度を考慮せずに翻訳が常に実行されます。

    テキスト速度が最速である場合、次の2つのパラメータを0に設定できます。そうでない場合、待機が必要な時間は次のパラメータによって決定されます：

1. #### 遅延（秒）

    固定の遅延時間を待機します（ゲームエンジンの内部ロジック処理を考慮して、0.1秒の固有の遅延があります）。

1. #### 画像安定性しきい値

    この値は、前述の同名のパラメータと似ています。ただし、これはテキストのレンダリングが完了したかどうかを判断するために使用されるため、上記の同名のパラメータと設定を共有しません。

    テキスト速度が遅い場合の予測不可能なレンダリング時間のため、指定された固定遅延では不十分な場合があります。画像と前のスクリーンショットの類似度がしきい値を超えると、アクションが実行されます。

## テキスト類似度しきい値

OCRの結果は不安定であり、画像のわずかな乱れがテキストにわずかな変化を引き起こし、繰り返し翻訳されることがあります。

各OCR呼び出し後、現在のOCR結果を前回のOCR結果（編集距離を使用）と比較します。編集距離がしきい値を超える場合にのみ、テキストが出力されます。
