# API スケッチ

C++ API は、ゼロベースで確定したデータモデル([DATA_MODEL.ja.md](DATA_MODEL.ja.md))に基づいて実装フェーズで設計します。キーの同一性(種別 + 値)のデータ表現などを実装判断として保留しているため([DECISIONS.ja.md](DECISIONS.ja.md))、確定 API はまだありません。

以下は方向性を示す雰囲気レベルの例で、確定 API ではありません。

```cpp
// UC1: 入れ替え(1 段 remap、連鎖しない)
config.remap(Key::CapsLock, Key::LeftCtrl);
config.remap(Key::LeftCtrl, Key::CapsLock);

// UC9: 種別またぎ remap
config.remap(Key::F13, Consumer::VolumeUp);

// UC5: per-input のレイアウト変換(on/off 切替キー付き)
config.input(0).convertLayout(Layout::enUs, /* host = */ Layout::jaJp);
config.remap(Key::F14, Virtual::LayoutConvertToggle);

// UC7 + レイヤー: ペダルを Fn に、押下中だけ HJKL を矢印に
config.remap(Key::F15, Virtual::Fn1);
config.layer(Virtual::Fn1).remap(Key::H, Key::Left);

// UC10: 文字列マクロ(出力の縁でホストレイアウトへ展開される)
config.textMacro(Virtual::Fn2, "user@example.com");
```

API 設計時の制約(データモデル由来):

- ドキュメントのコード例では namespace を省略しない方針(既存方針を踏襲)。
- core は Arduino / ESP-IDF 非依存・時刻なし・動的確保なし([CORE_DESIGN.ja.md](CORE_DESIGN.ja.md) の依存境界)。
- 設定は C++ 決め打ちと外部設定 object の両方を許容(Configuration Boundary)。
