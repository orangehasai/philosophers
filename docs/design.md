# philosophers 設計方針

この設計書は **Mandatory part (`philo`)** 前提です。  
Bonus は Mandatory を安定させてから別設計に切り出します。

## 1. 今回採用する設計

今回の設計方針は次です。

- 哲学者 1 人 = thread 1 本
- フォーク 1 本 = mutex 1 本
- 監視役 = メインスレッド
- 死亡判定 = メインスレッドが全哲学者を巡回
- デッドロック回避 = resource hierarchy
- 初手の競合緩和 = 軽い stagger
- `1 philosopher` は完全に別ケースとして扱う
- 時刻比較は `us` 基準、ログ表示だけ `ms`
- `meal_mutex` ではなく `state_mutex` を使う
- `print_mutex` と `stop_mutex` は分ける

この設計を選ぶ理由:

- 監視責任者を 1 箇所に固定できる
- fork 配列と相性が良い
- deadlock 回避を説明しやすい
- 死亡判定とログの競合を潰しやすい
- starvation-free の理論保証までは狙わない

## 2. 先に固定する不変条件

この課題では API より先に、不変条件を固定した方が壊れません。

- 1 本のフォークを同時に 2 人が使わない
- `last_meal_us` は無ロックで読まない / 書かない
- `meals_eaten` は無ロックで読まない / 書かない
- `stop` は無ロックで読まない / 書かない
- `died` は 1 回だけ出す
- `died` のあとに他ログを出さない
- `1 philosopher` では食事は絶対に起きない

## 3. データ構造

### 3.1 全体状態

```c
typedef struct s_rules
{
	int				num_philo;
	long long		time_to_die_ms;
	long long		time_to_eat_ms;
	long long		time_to_sleep_ms;
	int				must_eat_count;
	long long		start_us;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	*forks;
	struct s_philo	*philos;
}	t_rules;
```

### 3.2 哲学者ごとの状態

```c
typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_us;
	pthread_t		thread;
	pthread_mutex_t	state_mutex;
	int				left_fork_id;
	int				right_fork_id;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_rules			*rules;
}	t_philo;
```

## 4. 各フィールドの意味

### `t_rules`

- `num_philo`
  - 哲学者数。フォーク数でもある
- `time_to_die_ms`
  - 最後に食事開始してから何 ms で死ぬか
- `time_to_eat_ms`
  - eating 状態を維持する時間
- `time_to_sleep_ms`
  - sleeping 状態を維持する時間
- `must_eat_count`
  - 任意引数。未指定なら `-1`
- `start_us`
  - シミュレーション開始時刻。内部比較は `us`
- `stop`
  - 全体終了フラグ
- `stop_mutex`
  - `stop` を守る
- `print_mutex`
  - 出力整列と `died` 後ログ抑止を守る
- `forks`
  - 全フォーク実体
- `philos`
  - 全哲学者配列

### `t_philo`

- `id`
  - 1 始まりの哲学者番号
- `meals_eaten`
  - 食事完了回数
- `last_meal_us`
  - 最後に食事を開始した時刻
- `thread`
  - この哲学者の thread
- `state_mutex`
  - `last_meal_us`, `meals_eaten` を守る
- `left_fork_id`, `right_fork_id`
  - resource hierarchy 用の fork 番号
- `left_fork`, `right_fork`
  - 共有 fork 配列のどれを左右に持つか
- `rules`
  - 全体状態への参照

## 5. mutex の責務

### `forks[i]`

役割:

- フォークそのもの
- `lock = 取る`
- `unlock = 置く`

守る対象:

- フォークの同時使用禁止

### `state_mutex`

役割:

- 各哲学者の状態を守る

守る対象:

- `last_meal_us`
- `meals_eaten`

### `stop_mutex`

役割:

- 全体終了状態を守る

守る対象:

- `rules->stop`

### `print_mutex`

役割:

- ログ出力を整列する
- `died` 後に通常ログが出る競合を防ぐ

守る対象:

- `printf` の直列化
- stop 判定と通常ログ抑止の整合性

## 6. lock のルール

mutex は「何を守るか」だけでは足りません。  
「どの順で取るか」も固定します。

### 6.1 基本ルール

- フォーク待ち中に `state_mutex` は持たない
- `state_mutex` は最短時間だけ持つ
- `stop_mutex` 単体で触ることはある
- 出力系では `print_mutex -> stop_mutex` の順だけ許す
- `stop_mutex -> print_mutex` の順では取らない

### 6.2 なぜ `print_mutex -> stop_mutex` に固定するのか

通常ログと死亡ログの競合を防ぐためです。

通常ログは:

1. `print_mutex` を取る
2. `stop_mutex` を取る
3. `stop == 0` なら print
4. `stop_mutex` を離す
5. `print_mutex` を離す

死亡ログは:

1. `print_mutex` を取る
2. `stop_mutex` を取る
3. `stop == 0` なら `stop = 1`
4. `died` を print
5. `stop_mutex` を離す
6. `print_mutex` を離す

こうすると、`died` の直後に他ログが割り込むのを避けやすいです。

## 7. フォークの割り当て

フォーク実体は配列で持ちます。

```c
pthread_mutex_t	forks[num_philo];
```

各哲学者には次を割り当てます。

```c
philos[i].left_fork_id = i;
philos[i].right_fork_id = (i + 1) % num_philo;
philos[i].left_fork = &forks[i];
philos[i].right_fork = &forks[(i + 1) % num_philo];
```

たとえば 5 人なら:

- philo 1: left = fork 0, right = fork 1
- philo 2: left = fork 1, right = fork 2
- philo 3: left = fork 2, right = fork 3
- philo 4: left = fork 3, right = fork 4
- philo 5: left = fork 4, right = fork 0

この「同じ fork を隣同士が共有して見る」ために `left_fork` と `right_fork` はポインタです。

## 8. デッドロック回避戦略

今回採用するのは **resource hierarchy** です。

### 8.1 候補比較

#### ウェイター方式

利点:

- 公平性を入れやすい
- starvation 対策を強くしやすい

欠点:

- Mandatory の API 制約で実装が重い
- 独自状態管理が増える

#### モニタ方式

利点:

- 抽象化はきれい

欠点:

- 許可関数の範囲だと煩雑
- 条件待ちがなく polling 寄りになりやすい

#### resource hierarchy

利点:

- fork 配列と非常に相性が良い
- deadlock 回避を説明しやすい
- 実装が短い
- 評価で見られる観点に十分強い

欠点:

- starvation-free の理論保証はない
- 公平性最適化ではない

### 8.2 今回この方式でよい理由

レビュー項目上、重要なのはまず:

- deadlock しない
- 死亡判定が正しい
- 死亡ログが遅れない
- 5 800 200 200 系で不自然に死なない

です。  
公平性保証まで入れるより、まず競合と時刻精度を硬くする方が効果が高いです。

### 8.3 具体ルール

各哲学者は、自分の左右フォークのうち

- 番号が小さい方を先に取る
- 番号が大きい方を後に取る

と固定します。

```c
static pthread_mutex_t	*first_fork(t_philo *philo)
{
	if (philo->left_fork_id < philo->right_fork_id)
		return (philo->left_fork);
	return (philo->right_fork);
}

static pthread_mutex_t	*second_fork(t_philo *philo)
{
	if (philo->left_fork_id < philo->right_fork_id)
		return (philo->right_fork);
	return (philo->left_fork);
}
```

### 8.4 それでも starvation は起こりうる

理論上は起こりえます
ただし今回の狙いは starvation-free 証明ではなく標準テストで偏って死なないそこそこ安定的な実装です。

そのため安定化の補助で次を入れます。

- 初手 stagger
- `us` ベースの時刻比較
- eating 開始 / death 判定競合の lock 制御

## 9. stagger 方針

初手の取り合いを少しだけずらします。

方針:

- 偶数番 thread はループ開始前に `time_to_eat / 2` だけ待つ
- 哲学者数が奇数のときは philosopher `1` も同様に少し待つ

例:

```c
if (philo->id % 2 == 0 || (philo->rules->num_philo % 2 == 1
		&& philo->id == 1))
	precise_sleep(philo->rules->time_to_eat_ms / 2, philo->rules);
```

目的:

- 全員が同時に 1 本目を取りに行くのを弱める
- 実運用上の starvation を減らす

これは deadlock 回避の本体ではなく、競合緩和です。

## 10. `1 philosopher` の扱い

これは通常ケースに混ぜません。別分岐にします。

流れ:

1. その唯一の fork を取る
2. `has taken a fork` を出す
3. `time_to_die` まで待つ
4. 監視側が death を出す
5. fork を戻して終了

理由:

- 2 本目の fork が存在しない
- 通常の「2 本取れたら eating」が絶対に成立しない

## 11. 時刻管理

内部時刻は `us` で持ちます。  
ログ表示だけ `ms` に変換します。

```c
long long	now_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000LL + tv.tv_usec);
}
```

```c
long long	elapsed_ms(t_rules *rules)
{
	return ((now_us() - rules->start_us) / 1000LL);
}
```

こうする理由:

- `ms` 切り捨て比較より精度が高い
- `time_to_die` 監視で有利

## 12. `precise_sleep` 方針

`time_to_eat` と `time_to_sleep` は 1 回の長い `usleep` で寝ません。  
刻み sleep にします。

```c
int	precise_sleep(long long duration_ms, t_rules *rules)
{
	long long	start_us;
	long long	target_us;

	start_us = now_us();
	target_us = duration_ms * 1000LL;
	while (!simulation_should_stop(rules))
	{
		if (now_us() - start_us >= target_us)
			return (0);
		usleep(200);
	}
	return (1);
}
```

返り値:

- `0`
  - 指定時間ぶん待ち切った
- `1`
  - `stop` を検知して途中で抜けた

理由:

- stop を早く検知したい
- 長い block を避けたい
- 10ms 制約に寄せたい

## 13. 状態遷移

哲学者 thread は基本的に次を繰り返します。

1. stop でなければ fork を取る
2. `state_mutex` を取る
3. `last_meal_us = now_us()`
4. `state_mutex` を離す
5. `is eating` を出す
6. `time_to_eat_ms` だけ待つ
7. 完走したときだけ `state_mutex` を取る
8. `meals_eaten++`
9. `state_mutex` を離す
10. fork を置く
11. `is sleeping` を出す
12. `time_to_sleep_ms` だけ待つ
13. `is thinking` を出す
14. 哲学者数が奇数で `2 * time_to_eat - time_to_sleep > 0` のときは
    その分だけ追加で待つ

### 13.1 なぜ eating 開始前に `last_meal_us` を更新するのか

課題文上、死亡判定は「最後の食事開始から `time_to_die`」です。  
だから食べ終わりではなく **食べ始め** を記録します。

## 14. 哲学者状態を何で守るか

`state_mutex` で守る対象は次です。

- `last_meal_us`
- `meals_eaten`

```c
void	begin_eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->state_mutex);
	philo->last_meal_us = now_us();
	pthread_mutex_unlock(&philo->state_mutex);
}

void	finish_eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->state_mutex);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->state_mutex);
}
```

`eat` は次の順でつなぎます。

```c
void	eat(t_philo *philo)
{
	begin_eating(philo);
	print_state(philo, "is eating");
	if (precise_sleep(philo->rules->time_to_eat_ms, philo->rules) == 0)
		finish_eating(philo);
}
```

これで `stop` による早期終了時に
`meals_eaten` を増やさずに済みます。

## 15. stop 判定 helper

`stop` は全体共有なので helper 経由に統一します。

```c
int	simulation_should_stop(t_rules *rules)
{
	int	stop;

	pthread_mutex_lock(&rules->stop_mutex);
	stop = rules->stop;
	pthread_mutex_unlock(&rules->stop_mutex);
	return (stop);
}

void	set_stop_flag(t_rules *rules, int value)
{
	pthread_mutex_lock(&rules->stop_mutex);
	rules->stop = value;
	pthread_mutex_unlock(&rules->stop_mutex);
}
```

## 16. ログ方針

通常ログは helper 1 本にまとめます。

```c
void	print_state(t_philo *philo, const char *msg)
{
	long long	ts;
	int			stop;

	pthread_mutex_lock(&philo->rules->print_mutex);
	pthread_mutex_lock(&philo->rules->stop_mutex);
	stop = philo->rules->stop;
	if (!stop)
	{
		ts = (now_us() - philo->rules->start_us) / 1000LL;
		printf("%lld %d %s\n", ts, philo->id, msg);
	}
	pthread_mutex_unlock(&philo->rules->stop_mutex);
	pthread_mutex_unlock(&philo->rules->print_mutex);
}
```

### 16.1 死亡ログ

死亡ログは監視側だけが出します。

```c
void	announce_death(t_philo *philo)
{
	long long	ts;

	pthread_mutex_lock(&philo->rules->print_mutex);
	pthread_mutex_lock(&philo->rules->stop_mutex);
	if (philo->rules->stop == 0)
	{
		philo->rules->stop = 1;
		ts = (now_us() - philo->rules->start_us) / 1000LL;
		printf("%lld %d died\n", ts, philo->id);
	}
	pthread_mutex_unlock(&philo->rules->stop_mutex);
	pthread_mutex_unlock(&philo->rules->print_mutex);
}
```

これで:

- `died` は 1 回だけ
- `died` のあとに通常ログが出にくい

を同時に狙えます。

## 17. 監視ループ

監視はメインスレッドで行います。

```text
while (stop していない)
    全哲学者を順に見る
        state_mutex を取る
        last_meal_us, meals_eaten を読む
        unlock
        death 条件なら announce_death
    must_eat があるなら全員満腹判定
    少しだけ usleep
```

### 17.1 死亡判定

基本式はこれです。

```text
now_us - last_meal_us > time_to_die_ms * 1000
```

### 17.2 死亡確認をどこで行うか

死亡確認には 2 つの役割があります。

- monitor が死亡を公表して `stop` を立てること
- 各 philosopher が「次の行動に進んでよいか」を自分で確かめること

ここでの基準は次です。

- `stop`
  - monitor が終了を公表したあとの全体フラグ
- philosopher 自身の死亡確認
  - `last_meal_us` と現在時刻を比べる局所判定

この 2 つは似ていますが役割が違います。  
`stop` は全体終了の公開、局所判定は「この philosopher が次へ進めるか」の確認です。

### 17.3 行動開始の判定基準

局所判定を置く場所は「次の状態遷移を始める直前」に固定します。

- `take_forks`, `eat`, `sleep`, `think` の開始前
- `pthread_mutex_lock` のような block しうる処理から戻った直後
- `last_meal_us` のように状態遷移を確定させる共有値を書き換える直前

理由:

- block 中に死ぬことがある
- lock から復帰した時点で状況が変わっていることがある
- eating 開始は `last_meal_us` 更新で確定するので、判定と更新を同じ
  critical section に入れる必要がある

逆に、すでに完了した行動の bookkeeping にはこの判定を足しません。

- `meals_eaten++`
  - eating 完了の記録であり、新しい行動開始ではない

### 17.4 lock できてもログしないケース

fork 待ち中に死亡した場合は、`pthread_mutex_lock` 自体は成功して戻ることが
あります。  
このときは:

- fork をすぐ unlock する
- `has taken a fork` は出さない

とします。

理由:

- lock 成功は「低レベルの mutex 取得」にすぎない
- ログは「有効な状態遷移を公開する行為」として扱いたい

つまり「fork を一瞬取れた」ことより、
「その philosopher がまだ次の行動を始めてよいか」を優先します。

### 17.5 完食判定

`must_eat_count != MUST_EAT_UNSET` のときだけ有効です。  
全員 `meals_eaten >= must_eat_count` になれば stop します。

### 17.6 監視間隔

ループ末尾で `usleep(500)` か `usleep(1000)` を入れます。

理由:

- CPU を燃やしすぎない
- 10ms 制約には十分細かい

## 18. 初期化順

1. 引数パース
2. `t_rules` 設定
3. `forks` 配列 malloc
4. `philos` 配列 malloc
5. `stop_mutex` init
6. `print_mutex` init
7. 全 fork mutex init
8. 各 philo `state_mutex` init
9. `left_fork` / `right_fork` 接続
10. `start_us = now_us()`
11. 全 philo `last_meal_us = start_us`
12. thread 作成

## 19. 終了処理

1. 監視ループ終了
2. 全 thread `join`
3. 各 philo `state_mutex` destroy
4. 各 fork mutex destroy
5. `print_mutex` destroy
6. `stop_mutex` destroy
7. `philos` / `forks` free

重要:

- thread が動いている間に mutex を destroy しない
- stop を立ててから join
- join 後に destroy

## 20. ファイル分割案

- `main.c`
  - `main`
- `parse.c`
  - 引数パース
- `init.c`
  - rules / philos / forks 初期化
- `routine.c`
  - `1 philosopher` 分岐
  - philosopher routine
  - thread start helper
- `action_fork.c`
  - take forks / put forks
- `action_eat.c`
  - begin eating / finish eating / eat
- `action_sleep.c`
  - `philo_sleep`
- `action_think.c`
  - `think`
- `monitor.c`
  - death / full 監視
- `print.c`
  - 通常ログ / death ログ
- `time.c`
  - `now_us`, `elapsed_ms`, `precise_sleep`
- `stop.c`
  - `simulation_should_stop`, `set_stop_flag`
- `cleanup.c`
  - destroy / free

## 21. 実装順

1. 引数パース
2. `now_us`
3. stop helper
4. `print_state`
5. forks / philos 初期化
6. `1 philosopher` 分岐
7. philosopher routine の枠
8. fork 取得 / 解放
9. `begin_eating` / `finish_eating`
10. `precise_sleep`
11. monitor loop
12. must_eat 対応
13. cleanup

## 22. この設計の利点と欠点

### 利点

- deadlock 回避が明快
- 死亡判定責任者が 1 箇所
- `died` 後ログ競合を説明しやすい
- `state_mutex` 導入で哲学者状態の責務がはっきりする
- `us` 比較で時刻まわりが安定する

### 欠点

- starvation-free の理論保証はない
- waiter 方式ほど公平性は強くない
- `print_mutex -> stop_mutex` の lock 順を崩すと事故る

ただし今回の目標は「Mandatory を安全に通すこと」です。  
この設計は、その目的に対して十分に硬いです。
