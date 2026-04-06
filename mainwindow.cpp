#include "mainwindow.h"
#include "./ui_mainwindow.h"
#define DELAY_SHOW_MOLE_HIT_TIME 100
#define MOLE_APPEAR_SIMPLE_TIME 800
#define MOLE_APPEAR_HARD_TIME 400
#define MOLE_APPEAR_TIMEOUT 1000
#define GAME_LEFT_TIME 30

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_score(0)
    , m_timeLeft(GAME_LEFT_TIME)  // 游戏时长30秒
    , m_currentMoleIndex(-1)
    , m_previousMoleIndex(-1)
    , m_isHardMode(false)  // 默认简单
{
    ui->setupUi(this);
    initUI(); // 初始化界面

    // 初始化定时器
    m_moleTimer = new QTimer(this);
    m_gameTimer = new QTimer(this);
    m_hit_mole_timer = new QTimer(this);
    // 绑定信号槽
    connect(m_moleTimer, &QTimer::timeout, this, &MainWindow::moleAppear);
    connect(m_gameTimer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(m_hit_mole_timer, &QTimer::timeout, this, &MainWindow::updateMoleIcon);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化游戏界面
void MainWindow::initUI()
{
    setWindowTitle("砸地鼠定制版");
    //setFixedSize(440, 520);

    // 背景
    this->setStyleSheet(R"(
        QMainWindow {
            background-image:url(:background.png);
        }
    )");

    // 音效初始化
   // m_hitSound = new QSoundEffect(this);
   // m_hitSound->setSource(QUrl::fromLocalFile("hit.wav")); // 可以换成资源 :/hit.wav

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QGridLayout *layout = new QGridLayout(centralWidget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);
    layout->setContentsMargins(20,20,20,20);
    QHBoxLayout *hb_layout = new QHBoxLayout(this);
    // 手机适配关键让网格自动拉满
    for(int i =0; i < 4; i++){
        layout->setColumnStretch(i,1);
    }

    for(int i =0; i < 4; i++){
        layout->setRowStretch(i,1);
    }

    // 分数 时间
    m_scoreLabel = new QLabel("分数: 0", this);
    m_timeLabel = new QLabel("时间: 30", this);

    QString labelStyle = "color:blue; font-size:18px; font-weight:bold;";
    m_scoreLabel->setStyleSheet(labelStyle);
    m_timeLabel->setStyleSheet(labelStyle);

    // layout->addWidget(m_scoreLabel, 0,0);
    // layout->addWidget(m_timeLabel, 0,2);
    hb_layout->addWidget(m_scoreLabel);
    hb_layout->addWidget(m_timeLabel);
    hb_layout->setAlignment(Qt::AlignHCenter);;
    layout->addLayout(hb_layout,0,0,1,3);
    // 难度按钮
    m_easyBtn = new QPushButton("简单", this);
    m_hardBtn = new QPushButton("困难", this);

    m_easyBtn->setStyleSheet("background:#4caf50; color:white; padding:8px;");
    m_hardBtn->setStyleSheet("background:#ff9800; color:white; padding:8px;");

    // layout->addWidget(m_easyBtn, 0,1);
    // layout->addWidget(m_hardBtn, 0,3);
    hb_layout->addWidget(m_easyBtn);
    hb_layout->addWidget(m_hardBtn);

    connect(m_easyBtn, &QPushButton::clicked, this, &MainWindow::easyBtnClick);
    connect(m_hardBtn, &QPushButton::clicked, this, &MainWindow::hardBtnClick);

    // 9个地鼠按钮
    for(int i=0; i<9; i++){
        QPushButton *btn = new QPushButton(this);
        // 手机适配：按钮大小自适应，不固定
        btn->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        // 最小尺寸, 防止太小
        btn->setMinimumSize(80,80);

        btn->setStyleSheet(R"(
            QPushButton {
                border:none;
                border-radius: 8px;
                background-image: url(:/un_mole.png);
                background-repeat: no-repeat;
                background-position: center;
                background-size: contain;
                padding: 10px;
                box-shadow: 0 0 18px 5px rgba(255,255,0,0);
            }
            QPushButton:disabled {
                background-image: none;
                background-color: #d0d0d0;
                box-shadow: none;
            }
        )");

        btn->setEnabled(false);
        connect(btn, &QPushButton::clicked, this, &MainWindow::whackMole);
        m_moleBtns.append(btn);
        layout->addWidget(btn, i/3+1, i%3);
    }

    // 开始游戏
    m_restartBtn = new QPushButton("开始游戏", this);
    m_restartBtn->setStyleSheet(R"(
        QPushButton {
            font-size:16px; padding:8px;
            background-color:#2196f3; color:white; border-radius:6px;
        }
        QPushButton:hover {
            background-color:#64b5f6;
        }
    )");
    m_restartBtn->setEnabled(true);
    connect(m_restartBtn, &QPushButton::clicked, this, &MainWindow::startGame);
    layout->addWidget(m_restartBtn, 4,0,1,4);
}

// 开始游戏
void MainWindow::startGame()
{
    // 游戏中禁止点难度
    m_restartBtn->setText("重新开始游戏");
    m_easyBtn->setEnabled(false);
    m_hardBtn->setEnabled(false);
    m_restartBtn->setEnabled(false);

    m_score = 0;
    m_timeLeft = GAME_LEFT_TIME;
    m_scoreLabel->setText("分数: 0");
    m_timeLabel->setText("时间: 30");

    // 难度控制速度
    if(m_isHardMode)
        m_moleTimer->start(MOLE_APPEAR_HARD_TIME);  // 困难：超快
    else
        m_moleTimer->start(MOLE_APPEAR_SIMPLE_TIME);  // 简单

    m_gameTimer->start(MOLE_APPEAR_TIMEOUT);
}

// 地鼠随机出现
void MainWindow::moleAppear()
{
    // 先隐藏上一个地鼠
    if(m_currentMoleIndex != -1){
        m_moleBtns[m_currentMoleIndex]->setStyleSheet(R"(
            QPushButton {
                border:none;
                border-radius: 8px;
                background-image: url(:/un_mole.png);
                background-repeat: no-repeat;
                background-position: center;
                background-size: contain;
                padding: 10px;
                box-shadow: 0 0 18px 5px rgba(255,255,0,0);
            }
            QPushButton:disabled {
                background-image: none;
                background-color: #d0d0d0;
                box-shadow: none;
            }
        )");

        m_moleBtns[m_currentMoleIndex]->setEnabled(false);
    }

    // 随机生成新地鼠位置
    m_currentMoleIndex = QRandomGenerator::global()->bounded(9);
    m_moleBtns[m_currentMoleIndex]->setStyleSheet(R"(
            QPushButton {
                border:none;
                border-radius: 8px;
                background-image: url(:/mole.png);
                background-repeat: no-repeat;
                background-position: center;
                background-size: contain;
                padding: 10px;
                box-shadow: 0 0 18px 5px rgba(255,255,0,0);
            }
            QPushButton:disabled {
                background-image: none;
                background-color: #d0d0d0;
                box-shadow: none;
            }
        )");

    m_moleBtns[m_currentMoleIndex]->setEnabled(true);
}

// 砸地鼠（点击判定）
void MainWindow::whackMole()
{
    m_score += 10; // 砸中+10分
    m_scoreLabel->setText(QString("分数: %1").arg(m_score));

    m_previousMoleIndex = m_currentMoleIndex;
    // 砸中后隐藏
    m_moleBtns[m_currentMoleIndex]->setEnabled(false);
    // 拿到当前地鼠按钮
    QPushButton *mole = m_moleBtns[m_currentMoleIndex];
    // 设置【被砸中图片】
    mole->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 8px;
            background-image: url(:/mole_hit.png);
            background-repeat: no-repeat;
            background-position: center;
            background-size: contain;
            padding: 10px;
        }
    )");

    // 延迟 100ms 再隐藏，让玩家看到击中效果
    if(m_isHardMode == false){
        m_hit_mole_timer->start(DELAY_SHOW_MOLE_HIT_TIME);
    }else{
        m_hit_mole_timer->start(DELAY_SHOW_MOLE_HIT_TIME);
    }
    mole = nullptr;
}

// 倒计时更新
void MainWindow::updateTimer()
{
    m_timeLeft--;
    m_timeLabel->setText(QString("时间: %1").arg(m_timeLeft));
    // 低于10秒变红
    if(m_timeLeft <= 10) {
        m_timeLabel->setStyleSheet("color:red; font-size:17px; font-weight:bold;");
    }
    // 时间到，游戏结束
    if(m_timeLeft <= 0){
        m_moleTimer->stop();
        m_gameTimer->stop();
        // 隐藏所有地鼠
        for(int nx =0; nx < m_moleBtns.size(); nx++){
            m_moleBtns[nx]->setEnabled(false);

        }
        // 设置重新开始按钮显示
        m_restartBtn->show();
        // 设置游戏结束可以点击
        m_restartBtn->setEnabled(true);
        m_easyBtn->setEnabled(true);
        m_hardBtn->setEnabled(true);
        // 重新开始游戏
        m_restartBtn->setText("重新开始游戏");

        QMessageBox::information(this, "游戏结束", QString("最终得分：%1").arg(m_score));
    }
}

// 更新被击中的图片
void MainWindow::updateMoleIcon()
{
    // 隐藏时恢复默认样式（下一轮才会正常显示地鼠）
    m_hit_mole_timer->stop();
    m_moleBtns[m_previousMoleIndex]->setStyleSheet(R"(
            QPushButton {
                border:none;
                border-radius: 8px;
                background-image: url(:/un_mole.png);
                background-repeat: no-repeat;
                background-position: center;
                background-size: contain;
                padding: 10px;
                box-shadow: 0 0 18px 5px rgba(255,255,0,0);
            }
            QPushButton:disabled {
                background-image: none;
                background-color: #d0d0d0;
                box-shadow: none;
            }
        )");
}
//
void MainWindow::easyBtnClick()
{
    m_isHardMode = false;
}

//
void MainWindow::hardBtnClick()
{
    m_isHardMode = true;
}