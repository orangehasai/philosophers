# philo 実装タスク分割

このタスク分割は **Mandatory part (`philo`)** 前提です。  
目的は次の 2 つです。

- 1 タスクあたりのレビューを **20 分程度** で終えられる粒度にする
- 機能的に意味のある単位で分割し、途中で壊れにくくする

## 分割ルール

- 1 タスク = 1 つの責務
- 1 タスクの差分は、原則として 1〜3 ファイルに収める
- ただし Norm の 5 関数制限を避けるため、
  行動系 helper は `action_*.c` へ分割してよい
- 1 タスク完了時点で、少なくとも「壊れていない説明可能な状態」にする
- 並行処理の核心ロジックは、初期化・ログ・監視・行動を分けてレビューする
- Bonus はこの一覧に含めない

## T01 Makefile とファイル骨格の確定

- 目的:
  - 実装前に `philo/` 配下のファイル構成と `Makefile` の責務を固定する
- 主な対象:
  - `philo/Makefile`
  - `philo/philo.h`
  - 空の `.c` ファイル群
- 完了条件:
  - `make` が通る最小骨格がある
  - ソースファイル分割方針が [design.md](/Users/takenakatakeshiichirouta/Desktop/ft/level_4/philosophers/docs/design.md:633) と一致している
- レビュー観点:
  - ファイル責務が分かれているか
  - `Makefile` の変数名、ルール名、依存関係に破綻がないか
- 依存:
  - なし

## T02 引数パースと入力検証

- 目的:
  - CLI 引数を正しく `rules` 用の値へ変換し、不正入力を落とす
- 主な対象:
  - `parse.c`
  - `main.c`
  - `philo.h`
- 完了条件:
  - 引数個数チェックがある
  - 数値変換がある
  - `0`、負数、オーバーフロー系入力を弾ける
  - `must_eat_count` 未指定時は `-1` で扱える
- レビュー観点:
  - エラー処理が一貫しているか
  - `atoi` 相当実装が未定義動作寄りになっていないか
- 依存:
  - T01

## T03 時刻ユーティリティと stop helper

- 目的:
  - `us` 基準の内部時刻 API と、stop 参照 API を先に固定する
- 主な対象:
  - `time.c`
  - `stop.c` または `utils.c`
  - `philo.h`
- 完了条件:
  - `now_us`
  - `elapsed_ms`
  - `simulation_should_stop`
  - `set_stop_flag`
  が揃っている
- レビュー観点:
  - 単位が `us` / `ms` で混ざっていないか
  - stop の読み書きが helper に閉じているか
- 依存:
  - T01

## T04 データ構造と初期化

- 目的:
  - `t_rules`, `t_philo`, `forks`, `philos` のメモリ配置を作る
- 主な対象:
  - `init.c`
  - `philo.h`
- 完了条件:
  - `forks` 配列確保
  - `philos` 配列確保
  - `left_fork` / `right_fork` 接続
  - `left_fork_id` / `right_fork_id` 設定
  - `start_us`, `last_meal_us` 初期化
- レビュー観点:
  - 配列とポインタの接続が正しいか
  - 5 人時の fork 関係を説明できるか
- 依存:
  - T02
  - T03

## T05 mutex 初期化と cleanup 骨格

- 目的:
  - `forks`, `state_mutex`, `stop_mutex`, `print_mutex` の生成破棄を固める
- 主な対象:
  - `init.c`
  - `cleanup.c`
- 完了条件:
  - 全 mutex の init がある
  - 全 mutex の destroy がある
  - 途中失敗時の rollback 方針がある
- レビュー観点:
  - init と destroy の順が対応しているか
  - thread 稼働前提で壊れない cleanup か
- 依存:
  - T04

## T06 ログ層

- 目的:
  - 通常ログと死亡ログの責務を分けて、stop 後ログ抑止を固定する
- 主な対象:
  - `print.c`
  - `philo.h`
- 完了条件:
  - `print_state`
  - `announce_death`
  があり、`print_mutex -> stop_mutex` の順で統一されている
- レビュー観点:
  - `died` のあとに通常ログが出ない設計になっているか
  - ログ責任者が明確か
- 依存:
  - T03
  - T05

## T07 fork 取得順 helper

- 目的:
  - resource hierarchy に基づく fork 取得順を固定する
- 主な対象:
  - `action_fork.c`
  - `philo.h`
- 完了条件:
  - `first_fork`
  - `second_fork`
  - fork lock / unlock helper
  がある
- レビュー観点:
  - 小さい fork id を先に取る実装になっているか
  - 取得順と解放順が説明可能か
- 依存:
  - T04
  - T06

## T08 `1 philosopher` 分岐

- 目的:
  - 単独哲学者ケースを通常ロジックから分離する
- 主な対象:
  - `routine.c`
  - `main.c`
- 完了条件:
  - 1 本だけ fork を取る
  - eating に進まない
  - death 待ちになる
- レビュー観点:
  - 同じ fork を 2 回 lock しないか
  - 通常ケースに混ざっていないか
- 依存:
  - T06
  - T07

## T09 philosopher routine 骨格と stagger

- 目的:
  - 各哲学者 thread のループ枠を作り、初手 stagger を入れる
- 主な対象:
  - `routine.c`
  - `philo.h`
- 完了条件:
  - thread 関数がある
  - 偶数番 stagger がある
  - stop 判定でループを抜けられる
- レビュー観点:
  - 行動順の骨格が正しいか
  - stagger が deadlock 回避本体と混同されていないか
- 依存:
  - T07
  - T08

## T10 eating / sleeping / thinking の実装

- 目的:
  - philosopher の 1 周分の行動を完成させる
- 主な対象:
  - `action_eat.c`
  - `action_sleep.c`
  - `action_think.c`
  - `time.c`
  - `routine.c`
- 完了条件:
  - `begin_eating`
  - `finish_eating`
  - `precise_sleep`
  - `eat`
  - `philo_sleep`
  - `think`
  がつながっている
- レビュー観点:
  - `last_meal_us` 更新タイミングが eating 開始になっているか
  - `meals_eaten` の加算タイミングが eating 完了後か
  - `precise_sleep` が `us` 比較になっているか
- 依存:
  - T03
  - T06
  - T09

## T11 監視ループ

- 目的:
  - メインスレッドが死亡判定と完食判定を担う形を完成させる
- 主な対象:
  - `monitor.c`
  - `main.c`
- 完了条件:
  - 各 philosopher の `last_meal_us` を読む
  - `time_to_die_ms` 超過を検出する
  - `must_eat_count` 到達時に正常終了できる
- レビュー観点:
  - death 判定責任者が 1 箇所か
  - `died` が monitor からのみ出るか
  - polling 間隔が粗すぎないか
- 依存:
  - T10

## T12 thread 起動 / join / 全体接続

- 目的:
  - `main` から初期化、thread 起動、監視、join、cleanup まで通す
- 主な対象:
  - `main.c`
  - `init.c`
  - `cleanup.c`
- 完了条件:
  - thread 作成
  - monitor 実行
  - stop 後 join
  - cleanup 完了
- レビュー観点:
  - destroy が join 後になっているか
  - エラー経路でも解放漏れがないか
- 依存:
  - T05
  - T11

## T13 標準シナリオ確認

- 目的:
  - レビュー票に寄せた主要ケースの手動確認を揃える
- 主な対象:
  - 実行確認
  - 必要なら軽微な修正
- 完了条件:
  - `1 800 200 200`
  - `2 800 200 200`
  - `5 800 200 200`
  - `5 800 200 200 7`
  - `4 410 200 200`
  を見て、死ぬべき/死ぬべきでないが合っている
- レビュー観点:
  - starvation 的な偏りが標準テストで出ないか
  - `died` 後ログがないか
  - 2 philosopher の death timing が大きく遅れないか
- 依存:
  - T12

## T14 Norm / README / 提出整理

- 目的:
  - 実装後の提出品質を固める
- 主な対象:
  - `README.md`
  - `philo/`
  - 不要ファイル整理
- 完了条件:
  - README の必須項目が埋まる
  - 提出物が整理される
  - Norm 観点の違反が潰れている
- レビュー観点:
  - 提出対象に不要物が混ざっていないか
  - 説明文と実装がズレていないか
- 依存:
  - T13

## レビューしやすさの目安

各タスクの diff は、目安として次に収めます。

- 主要責務は 1 つ
- 変更ファイルは 1〜3 個
- コアロジックは 150 行前後まで
- 並行処理の危険箇所は 1 タスクに 1 種類まで

この制約を守ると、レビュー側が

- 何を見ればよいか
- 何が今回の責務か
- どこに競合リスクがあるか

を 20 分前後で追いやすくなります。
