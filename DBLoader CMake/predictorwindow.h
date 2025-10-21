#ifndef PREDICTORWINDOW_H
#define PREDICTORWINDOW_H

#include <QMainWindow>
#include "loaderwidnow.h"
#include "NeuralNetwork.h"
#include <QCloseEvent>
#include <QTimer>
#include <atomic>

namespace Ui {
class PredictorWindow;
}

class PredictorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PredictorWindow(QWidget *parent = nullptr);
    ~PredictorWindow();
    LoaderWidnow* sourceWindow = nullptr;
    NeuralNetwork nn;
    bool stopRequested = false;


    int currentIteration = 0;
    int iterationCount = 0;


    int firstInputColumn = 0;
    int lastInputColumn = 0;
    int firstOutputColumn = 0;
    int lastOutputColumn = 0;
    int inputSize = lastInputColumn - firstInputColumn + 1;
    int outputSize = lastOutputColumn - firstOutputColumn + 1;

    std::vector<std::vector<float>> inputs;
    std::vector<std::vector<float>> outputs;

    std::vector<std::map<QString,float>> stringValue;

    std::vector<int> incorrectInputPredictions;
    std::vector<int> incorrectInputPredictionsBuffer;

    std::vector <bool> textConvertion;

    int onlyErrorIterationCount = 0;
    int onlyErrorIterationCountMax = 5;

    QTimer exectimer;
    bool first_iter=true;
    bool timer_training = false;
    int cost = 0;
    float learningRate = 0.0001f;

    void train();

    void processData();
    void updateSourceData();

    void closeEvent(QCloseEvent* event) override;
private slots:

    void timerTick();

    void on_browse_pressed();

    void on_loadPushButton_pressed();

    void on_savePushButton_pressed();

    void on_trainPushButton_clicked();

    void on_predictPushButton_pressed();

    void on_stopPushButton_pressed();

    void on_columnsToIgnoreSlider_valueChanged(int value);

    void on_inputCountSlider_valueChanged(int value);

    void on_outputCountSlider_valueChanged(int value);

    void on_hiddenLayerCountSlider_valueChanged(int value);

    void on_hiddenLayerSizeSlider_valueChanged(int value);

    void on_recreatePushButton_pressed();

private:
    Ui::PredictorWindow *ui;
};

#endif // PREDICTORWINDOW_H
