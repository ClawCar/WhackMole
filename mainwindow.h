#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QVector>
#include <QGridLayout>
#include <QRandomGenerator>
#include <QMessageBox>
//#include <QSoundEffect>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private slots:
    void startGame();      // 开始游戏
    void moleAppear();     // 地鼠随机出现
    void whackMole();      // 砸地鼠
    void updateTimer();    // 更新倒计时
    void easyBtnClick();
    void hardBtnClick();
    void updateMoleIcon();    // 更新被击中的图片
private:
    void initUI();         // 初始化界面
    Ui::MainWindow *ui;
    QVector<QPushButton*> m_moleBtns; // 地鼠按钮数组
    QLabel* m_scoreLabel;   // 分数标签
    QLabel* m_timeLabel;    // 时间标签
    QTimer* m_moleTimer;    // 地鼠出现定时器
    QTimer* m_gameTimer;    // 游戏倒计时定时器
    QPushButton* m_restartBtn; // 重新开始按钮
    QTimer* m_hit_mole_timer;  // 砸中地鼠之后,恢复之前为砸中的图片样式
    QPushButton *m_easyBtn; // 简单模式
    QPushButton *m_hardBtn; // 困难模式
    bool m_isHardMode;      // 当前设置的难度
   // QSoundEffect *m_hitSound;  // 击中音效

    int m_score;     // 分数
    int m_timeLeft;  // 剩余时间
    int m_currentMoleIndex; // 当前显示的地鼠索引
    int m_previousMoleIndex; // 当前上一次显示的地鼠索引
};
#endif // MAINWINDOW_H
