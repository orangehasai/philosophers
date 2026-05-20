# philosophers 学習メモ

このドキュメントは、`philosophers` 童貞が、頭から読んで

- この課題が何をさせたいのか
- なぜその設計になるのか
- `pthread` / `mutex` / `usleep` が何者なのか
- どのデータを何で守るべきか

を順番に理解できるようにまとめたものです。

## 1. この課題は何をやらせたいのか

この課題の本質は「同時に動く複数人が、同じ資源を奪い合うとき、どう安全に制御するか」です。

表面上は「哲学者が食べる・寝る・考える」ですが、実際には次の 3 問が中心です。

- 複数の実行単位を同時に動かす
- 共有資源であるフォークを壊さずに取り合う
- 時間制限つきで死亡判定を行う

Mandatory part は `thread + mutex`、Bonus part は `process + semaphore` でこれをやります。

## 2. まず問題を分解して見る

まず登場人物を4つに分けると整理しやすいです。

- 哲学者
  - 動き続ける主体
- フォーク
  - 共有資源
- 監視役
  - 誰かが死んだかを確認する役
- 全体ルール
  - 開始時刻、終了条件、ログ出力順など

この分解をすると、自然に次の対応になります。

- 哲学者
  - thread または process
- フォーク
  - mutex または semaphore
- 監視役
  - 監視ループまたは監視スレッド
- 全体ルール
  - 共有構造体

## 3. なぜその設計になるのか

### 3.1 なぜ `struct` が必要なのか

哲学者 1 人が動くのに必要な情報は 1 個ではありません。

- 自分の番号
- 左フォーク
- 右フォーク
- 最後に食べた時刻
- 食事回数
- 全体設定への参照

これをバラバラに渡すと管理できないので、哲学者 1 人分の文脈を `struct` にまとめます。

つまり `struct` は「`pthread_create` の都合で使う箱」ではなく、

**哲学者 1 人が動くための情報のまとまり**です。

### 3.2 なぜロックが必要なのか

複数スレッドが同じデータを同時に触ると壊れるからです。

たとえば `last_meal_ms` は、

- 哲学者自身が更新する
- 監視側が読む

ので、両者が同時に触る可能性があります。  
ルールなしで読む / 書くと、結果が実行順に依存して不安定になります。

だから「このデータを触る前には必ずこの mutex を取る」と決めます。

### 3.3 なぜフォークが mutex になるのか

フォークに必要な性質は、ほぼ 1 つだけです。

- 同時に 1 人しか使えない

これは mutex の性質と一致しています。

対応関係はこうです。

- `pthread_mutex_lock(&forks[i])`
  - フォークを取る
- `pthread_mutex_unlock(&forks[i])`
  - フォークを置く

つまりこの課題では、mutex は「排他制御のための何か」であると同時に、

**フォークそのものを表す道具**

でもあります。

### 3.4 なぜ監視役が必要なのか

哲学者自身が常に自分を監視できるわけではないからです。

哲学者は次の場面で止まります。

- フォーク待ち
- 食事中
- 睡眠中

特に `pthread_mutex_lock` は、フォークが空くまでブロックします。  
その間、その哲学者スレッドは自分の死亡判定を進められません。

だから外から定期的に

- いま何時か
- 最後に食べたのはいつか
- `time_to_die` を超えたか

を見る役が必要です。

### 3.5 なぜ `usleep` を刻んで使うのか

課題文には「死亡ログは実際の死亡から 10ms 以内」とあります。  
もし `time_to_sleep = 200` をそのまま 1 回で長く寝ると、

- stop してもすぐ起きない
- 監視の粒度が粗くなる
- タイミングが遅れやすい

という問題が出ます。

だから

- CPU を無駄に燃やさず
- でも長く寝すぎず

という妥協として、短く `usleep` しながら経過時間を見る実装がよく使われます。

## 4. 最低限必要な OS / C の前提

### 4.1 process と thread

`process` は OS が管理する独立した実行単位です。  
`thread` は同じ process の中で動く軽量な実行単位です。

違いは共有メモリです。

- thread
  - 同じアドレス空間を共有する
- process
  - 普通の変数は共有しない

だから Mandatory は mutex で共有メモリを守ればよく、Bonus は semaphore や `kill` / `waitpid` が必要になります。

### 4.2 ブロッキングとは何か

「関数が返ってくるまでそのスレッドが先へ進めない」ことです。

例:

- `pthread_mutex_lock`
  - 相手がフォークを離すまで待つ
- `usleep`
  - 指定時間が経つまで待つ
- `pthread_join`
  - 対象スレッドが終わるまで待つ

この課題が難しいのは、「哲学者はブロッキングするのに、死亡判定は止まらない」からです。

### 4.3 時間の単位

この課題では単位が混ざりやすいです。

- `s`
  - 秒
- `ms`
  - millisecond, ミリ秒, `1/1000` 秒
- `us`
  - microsecond, マイクロ秒, `1/1000000` 秒

関係はこうです。

- `1 s = 1000 ms`
- `1 ms = 1000 us`

`usleep` の引数は `us` です。  
たとえば 200ms なら `200000 us` です。

## 5. `sleep` と `usleep`

### 5.1 違い

- `sleep(unsigned int seconds)`
  - 秒単位
- `usleep(useconds_t usec)`
  - マイクロ秒単位

例:

```c
sleep(1);       /* 1秒 */
usleep(1000);   /* 1ms */
usleep(500000); /* 0.5秒 */
```

この課題は ms 単位の制御が必要なので、`sleep` では粗すぎます。  
だから `usleep` を使います。

### 5.2 `usleep` は何をしているのか

`usleep` は「今のスレッドを最低これくらい止める」関数です。  
完全に正確なタイマーではありません。

意味合いとしては:

- CPU を明け渡す
- 他スレッドを動かせる
- 自分は少し待つ

です。

### 5.3 なぜ sleep が必要なのか

理論上は、こうやって待つこともできます。

```c
while (now_us() - start_us < target_us)
	;
```

でもこれは busy wait です。

- CPU を無駄に使う
- 他スレッドの実行機会を潰す
- かえって不安定になる

だから `usleep` で CPU を明け渡します。

### 5.4 刻み sleep は「短く寝ている」のか

この形を考えます。

```c
while (now_us() - start_us < target_us)
	usleep(200);
```

これは「睡眠状態をシミュレーション上維持しつつ、細かく経過時間を確認している」だけです。  
途中でフォークを取りに行ったり thinking に移ったりしないので、課題の意味ではちゃんと sleeping です。

厳密さを上げたいなら、比較は `ms` ではなく `us` で行う方が自然です。

```c
#include <sys/time.h>

long long	now_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000LL + tv.tv_usec);
}

void	precise_sleep(long long duration_ms)
{
	long long	start_us;
	long long	target_us;

	start_us = now_us();
	target_us = duration_ms * 1000LL;
	while (now_us() - start_us < target_us)
		usleep(200);
}
```

## 6. mutex をちゃんと理解する

### 6.1 mutex は何者か

`mutex` は `mutual exclusion` の略で、排他制御のためのロック装置です。

重要なのは、mutex は

**保護したいデータのアドレスを知っているわけではない**

ということです。

たとえば:

```c
pthread_mutex_t	lock;
int				counter;
```

このとき:

- `lock` は `counter` を知らない
- `counter` も `lock` を知らない

でも実装者が

- `counter` を触る前には必ず `lock` を取る

と決めることで、`counter` を守れます。

つまり mutex とデータの対応は、OS が自動で作るのではなく、

**実装者が勝手に関連付ける**

ものです。

### 6.2 OS から見ると何が起きているか

OS / pthread ライブラリが見ているのは概念的には次だけです。

- その mutex がロック中か
- 誰が持っているか
- 待っているスレッドがいるか

イメージとしてはこうです。

```c
typedef struct s_fake_mutex
{
	int	locked;
	int	owner;
	int	waiters;
}	t_fake_mutex;
```

もちろん本物の `pthread_mutex_t` はもっと複雑ですが、本質は似ています。

### 6.3 lock / unlock で何が起こるか

`pthread_mutex_lock(&m)` を呼ぶと、概念的にはこうです。

1. `m` が空いているか見る
2. 空いていれば自分が取る
3. 埋まっていれば待つ

`pthread_mutex_unlock(&m)` はこうです。

1. 自分のロックを外す
2. 待っているスレッドがいれば起こす

大事なのは、mutex はデータそのものを守るより、

**そのデータに触るコード区間を直列化している**

ということです。

### 6.4 宣言と初期化の違い

```c
pthread_mutex_t	lock;
```

これは「mutex 用のメモリ領域を置いた」だけです。  
まだ使える状態とは限りません。

```c
pthread_mutex_init(&lock, NULL);
```

で、その領域を使える mutex オブジェクトに初期化します。

流れはこうです。

```c
pthread_mutex_t	lock;

pthread_mutex_init(&lock, NULL);
pthread_mutex_lock(&lock);
pthread_mutex_unlock(&lock);
pthread_mutex_destroy(&lock);
```

### 6.5 1つの mutex で複数データを守れるのか

守れます。  
OS は「この mutex がどのデータを守るか」を知らないので、混同しません。

たとえば:

```c
typedef struct s_state
{
	int				stop;
	int				finished_count;
	long long		last_event_ms;
	pthread_mutex_t	state_mutex;
}	t_state;
```

この `state_mutex` を使って

- `stop`
- `finished_count`
- `last_event_ms`

をまとめて守れます。

```c
pthread_mutex_lock(&state->state_mutex);
state->stop = 1;
state->finished_count++;
state->last_event_ms = now_ms();
pthread_mutex_unlock(&state->state_mutex);
```

ただし、同じデータをたまに別 mutex で触ったり、ロックなしで触ったりすると壊れます。

### 6.6 mutex はデータと 1 対 1 でなければならないか

必須ではありません。

- 関連する複数データを 1 本で守る
- データごとに細かく分ける

のどちらもあり得ます。

`philosophers` では、初心者は責務を分けた方が見通しがよいです。

- フォークごとに 1 本
- print 用に 1 本
- stop 用に 1 本
- 各哲学者の meal 状態用に 1 本

## 7. フォークが mutex であることを具体的に見る

全フォーク実体は普通こう置きます。

```c
pthread_mutex_t	forks[5];
```

これは「mutex の配列」であると同時に、この課題では

**フォーク 5 本**

です。

各哲学者は、そのうちどの 2 本を左右フォークとして使うかを参照します。

```c
philos[0].left_fork = &forks[0];
philos[0].right_fork = &forks[1];

philos[1].left_fork = &forks[1];
philos[1].right_fork = &forks[2];
```

ここで重要なのは:

- `philos[0].right_fork`
- `philos[1].left_fork`

が、同じ `&forks[1]` を指すことです。  
これで「同じ 1 本のフォークを取り合う」が表現できます。

だから `left_fork` と `right_fork` はポインタです。

### 7.1 値で持ってはいけない理由

もしこうすると:

```c
pthread_mutex_t	left_fork;
```

これは「共有フォークを参照する」のではなく、「自分の中に別 mutex 実体を持つ」ことになります。  
それでは同じフォークを共有できません。

なので意味としては、

- 同じフォーク実体を共有参照したい

です。

## 8. 最小のデータ構造例

Mandatory part の最小イメージはこうです。

```c
typedef struct s_rules
{
	int				num_philo;
	long long		time_to_die;
	long long		time_to_eat;
	long long		time_to_sleep;
	long long		start_ms;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	print_mutex;
}	t_rules;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_ms;
	pthread_t		thread;
	pthread_mutex_t	meal_mutex;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	t_rules			*rules;
}	t_philo;
```

各フィールドの意味:

- `thread`
  - この哲学者の thread 識別子
- `meal_mutex`
  - `last_meal_ms` や `meals_eaten` を守る mutex
- `left_fork`
  - 左フォーク実体へのポインタ
- `right_fork`
  - 右フォーク実体へのポインタ

### 8.1 `meal_mutex` は何を守るのか

名前はただの責務のラベルです。  
この mutex で守りたいのが

- `last_meal_ms`
- `meals_eaten`

なら `meal_mutex` で自然です。

もし「哲学者の内部状態全般」を守るなら `state_mutex` でもよいです。  
大事なのは名前より、

**どのデータをこの mutex で守ると決めたかを一貫させること**

です。

## 9. `pthread` API を課題の文脈で理解する

### 9.1 `pthread_create`

新しい thread を作ります。

```c
static void	*philo_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	while (!simulation_should_stop(philo->rules))
	{
		take_forks(philo);
		eat(philo);
		put_forks(philo);
		philo_sleep(philo);
		think(philo);
	}
	return (NULL);
}
```

```c
int	start_threads(t_philo *philos, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_create(&philos[i].thread, NULL,
				philo_routine, &philos[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}
```

ポイント:

- 引数は `void *` なので `struct` を渡す
- `&philos[i]` を渡すので、その配列実体は生き続ける必要がある
- ループ変数 `i` のアドレスは渡してはいけない

### 9.2 `pthread_join`

thread の終了を待ちます。

```c
void	join_threads(t_philo *philos, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(philos[i].thread, NULL);
		i++;
	}
}
```

この課題では、最後に全員を確実に回収したいので `join` が扱いやすいです。

### 9.3 `pthread_mutex_init / destroy`

フォーク配列の初期化例です。

```c
int	init_forks(pthread_mutex_t *forks, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_mutex_init(&forks[i], NULL) != 0)
			return (1);
		i++;
	}
	return (0);
}
```

```c
void	destroy_forks(pthread_mutex_t *forks, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&forks[i]);
		i++;
	}
}
```

### 9.4 `pthread_mutex_lock / unlock`

フォーク取得の最小形です。

```c
void	take_forks(t_philo *philo)
{
	pthread_mutex_lock(philo->left_fork);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(philo->right_fork);
	print_state(philo, "has taken a fork");
}
```

ただしこれは、全員が同じ順だとデッドロックし得ます。  
実際には奇数偶数で順序を変えるなどの工夫が必要です。

### 9.5 `gettimeofday`

現在時刻を取得します。  
この課題では「開始から何 ms / us 経過したか」を計算するために使います。

```c
#include <sys/time.h>

long long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000LL + tv.tv_usec / 1000LL);
}
```

ログ用には通常こう使います。

```c
ts = now_ms() - rules->start_ms;
printf("%lld %d is thinking\n", ts, philo->id);
```

### 9.6 ログ出力用 mutex

`printf` を複数 thread が同時に呼ぶとログが混ざります。  
だから `print_mutex` を使います。

```c
void	print_state(t_philo *philo, const char *msg)
{
	long long	ts;

	pthread_mutex_lock(&philo->rules->print_mutex);
	ts = now_ms() - philo->rules->start_ms;
	printf("%lld %d %s\n", ts, philo->id, msg);
	pthread_mutex_unlock(&philo->rules->print_mutex);
}
```

実際には stop 後に余計なログを出さない条件も必要です。

## 10. 哲学者自身の状態は何で守るか

よく守る対象は次です。

- `last_meal_ms`
- `meals_eaten`

監視側が読むので、mutex が必要です。

```c
void	set_last_meal(t_philo *philo, long long now)
{
	pthread_mutex_lock(&philo->meal_mutex);
	philo->last_meal_ms = now;
	pthread_mutex_unlock(&philo->meal_mutex);
}

long long	get_last_meal(t_philo *philo)
{
	long long	last;

	pthread_mutex_lock(&philo->meal_mutex);
	last = philo->last_meal_ms;
	pthread_mutex_unlock(&philo->meal_mutex);
	return (last);
}
```

`meals_eaten` も監視や完食判定で触るなら同じ mutex で守るのが自然です。

## 11. stop フラグはなぜ別で守るのか

終了条件は全 thread から読まれ、監視側などから書かれます。  
これも共有データです。

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

## 12. 監視ループの最小形

監視側は「最後に食べた時刻」と「今の時刻」を比較します。

```c
int	check_death(t_philo *philos, int count)
{
	int			i;
	long long	now;

	i = 0;
	while (i < count)
	{
		now = now_ms();
		if (now - get_last_meal(&philos[i]) > philos[i].rules->time_to_die)
		{
			set_stop_flag(philos[i].rules, 1);
			print_state(&philos[i], "died");
			return (1);
		}
		i++;
	}
	return (0);
}
```

実際には次も考慮が必要です。

- 完食条件
- `died` 後に他ログを出さないこと
- 死亡ログを 10ms 以内に出すこと

## 13. デッドロックとその回避

全員が同じ順でフォークを取ると、こうなり得ます。

1. 全員が左フォークを取る
2. 全員が右フォーク待ちになる
3. 誰も右を取れない
4. 全員止まる

これがデッドロックです。

代表的な回避策はこれです。

- 奇数番
  - 左 -> 右
- 偶数番
  - 右 -> 左

つまり「全員が同じ順で 1 本目を取りに行く」状態を壊します。

## 14. 1 philosopher が別問題である理由

1 人しかいないとフォークも 1 本しかありません。  
でも食事には 2 本必要です。

なので通常ケースの

- 左右 2 本取れたら食べる

は成立しません。

このケースでは普通、

- 1 本取るログを出す
- `time_to_die` 経過後に死ぬ

という特別処理が必要です。

## 15. Bonus part で考え方が変わる点

Bonus では各哲学者が process になります。  
つまり普通の変数は共有できません。

そのため:

- Mandatory
  - 共有メモリ + mutex
- Bonus
  - process 間同期 + semaphore + `waitpid` + `kill`

に変わります。

### 15.1 `fork`

```c
pid_t	pid;

pid = fork();
if (pid < 0)
	return (1);
if (pid == 0)
{
	run_philosopher_process(philo);
	exit(0);
}
philo->pid = pid;
```

### 15.2 `waitpid`

```c
int		status;
pid_t	finished;

finished = waitpid(-1, &status, 0);
if (finished > 0)
{
	if (WIFEXITED(status))
		handle_child_exit(finished, WEXITSTATUS(status));
}
```

### 15.3 `kill`

```c
void	kill_all_children(t_philo *philos, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (philos[i].pid > 0)
			kill(philos[i].pid, SIGTERM);
		i++;
	}
}
```

### 15.4 semaphore

```c
sem_t	*forks;

sem_unlink("/philo_forks");
forks = sem_open("/philo_forks", O_CREAT, 0644, num_philos);
if (forks == SEM_FAILED)
	return (1);
```

```c
sem_wait(forks);
print_state_bonus(philo, "has taken a fork");
sem_wait(forks);
print_state_bonus(philo, "has taken a fork");
```

```c
sem_post(forks);
sem_post(forks);
```

Mandatory と違って、「フォーク mutex 配列を共有する」発想がそのまま使えないのが本質です。

## 16. 設計前に決めるべきこと

最低限これだけは先に決めた方がよいです。

- どのデータを共有するか
- どのデータをどの mutex で守るか
- 監視役を誰にするか
- デッドロックをどう回避するか
- `1 philosopher` をどう扱うか
- 完食条件あり / なしをどう止めるか

## 17. この課題でよく使う mutex の責務表

- `forks[i]`
  - フォーク `i`
- `print_mutex`
  - ログ出力
- `stop_mutex`
  - `stop` フラグ
- `meal_mutex` または `state_mutex`
  - `last_meal_ms`, `meals_eaten`

読み取り専用の設定値は普通 mutex 不要です。

- `id`
- `time_to_die`
- `time_to_eat`
- `time_to_sleep`

## 18. FAQ

### Q1. mutex は保護したいデータのアドレスを知っているのか

知りません。  
mutex は「ロック状態」を持っているだけです。

どのデータをその mutex で守るかは、実装者が決めます。

### Q2. 1 つの mutex で複数データを守ると、OS 的に混ざらないのか

混ざりません。  
OS は「この mutex が何のデータを守るか」を知りません。

OS から見ると、

- lock 中か
- 誰が持っているか
- 待っている人がいるか

だけです。

混ざるかどうかは OS ではなく、実装者が一貫して同じ mutex を使うかどうかです。

### Q3. フォーク自体が mutex というのがまだ不思議

この課題でのフォークに必要なのは「同時に 1 人しか使えない」ことだけです。  
だからその性質を mutex でそのまま表しています。

つまり

- lock = 取る
- unlock = 置く

です。

### Q4. `left_fork` / `right_fork` がポインタなのはなぜか

同じフォーク実体を複数哲学者で共有するためです。

```c
pthread_mutex_t forks[5];
```

が本物のフォークで、

```c
philos[0].right_fork = &forks[1];
philos[1].left_fork = &forks[1];
```

のように同じアドレスを指すことで、「同じ 1 本を取り合う」を表します。

### Q5. 哲学者自身が自分を監視できないのはなぜか

フォーク待ちや `usleep` で止まるからです。  
特に `pthread_mutex_lock` は相手がフォークを離すまで返ってこないので、その間は自分で死亡判定を進められません。

### Q6. 刻み sleep は許されるのか

許されます。  
重要なのは「その時間が経つまで次の行動に進まないこと」であって、カーネルの中で完全連続睡眠していることではありません。

刻み sleep は

- CPU を明け渡す
- でも stop や死亡検知を遅らせすぎない

ための実装です。

### Q7. `sleep` と `usleep` の違いは何か

- `sleep`
  - 秒単位
- `usleep`
  - マイクロ秒単位

この課題は ms 単位の制御が必要なので `usleep` を使います。

### Q8. `meal_mutex` は meal にしか使えないのか

いいえ。  
単なる名前です。

その mutex で守る対象が

- `last_meal_ms`
- `meals_eaten`

なら `meal_mutex` という名前が自然、というだけです。  
もし責務を広く取りたいなら `state_mutex` の方がわかりやすいこともあります。

## 19. 最後に

この課題で大事なのは、API を覚えることより次を固定することです。

- 何が共有データか
- 何を何で守るか
- 誰が監視するか
- どう止めるか

ここが固まると、コードはかなり書きやすくなります。
