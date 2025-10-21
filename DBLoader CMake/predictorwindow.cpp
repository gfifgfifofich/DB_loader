#include "predictorwindow.h"
#include "ui_predictorwindow.h"
#include <QFileDialog>
inline QString usrDir;
inline QString documentsDir;
inline DataStorage userDS;
inline DataStorage historyDS;


inline PredictorWindow* ptrPredictorWindow = nullptr;

PredictorWindow::PredictorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PredictorWindow)
{
    ui->setupUi(this);


    ui->saveNameLineEdit->setText(documentsDir + "/NN.nn");
    ui->stopPushButton->hide();

    exectimer.setInterval(16);
    exectimer.setSingleShot(false);
    exectimer.start();
    connect(&exectimer, SIGNAL(timeout()), this, SLOT(timerTick()));

}

void PredictorWindow::timerTick()
{
    ui->costProgressBar->setMaximum(inputs.size());
    ui->costProgressBar->setValue(abs(cost));

    ui->iterationProgressBar->setMaximum(iterationCount);
    ui->iterationProgressBar->setValue(currentIteration);



    if(timer_training)
    {
        if(inputs.size() <= 0)
            return;

        if(first_iter)
        {
            qDebug() << "CreateGPU";
            nn.CreateGPU(nn.Arch,nn.LayersAmount);
            nn.UpdateWeightsAndBiases();
            first_iter = false;
            qDebug() << "done";
        }
        if(stopRequested)
        {
            stopRequested = false;
            timer_training = false;
            return;
        }
        currentIteration++;
        int tmpcost = 0;
        // nn.UpdateGPUInput(inputs[i].data());
        // nn.UpdateGPUTargetOutput(outputs[i].data());
        //nn.BackPropagateGPU(inputs[i].data(),outputs[i].data(),learningRate);

        int buff_max_size = incorrectInputPredictions.size(); // fix incorrects
        bool go_on_incorrects = true;
        if(buff_max_size <= 0 || onlyErrorIterationCount > onlyErrorIterationCountMax)
        {
            incorrectInputPredictions.clear();
            buff_max_size = inputs.size();// learn on all
            go_on_incorrects = false;
            onlyErrorIterationCount=0;
        }
        int buff_size_processed = 0;
        int buff_size_left = buff_max_size;

        int butch_size = 1000;

        bool setupped_for_butch_size = false;


        while (buff_size_left > 0)
        {
            int current_batch_size = 1;

            if(buff_size_left >= butch_size)
            {
                buff_size_left -= butch_size;
                current_batch_size = butch_size;
            }
            else
            {
                current_batch_size = buff_size_left;
                buff_size_left = 0;
            }
            std::vector<float> inputs_amalgamation;
            std::vector<float> outputs_amalgamation;
            inputs_amalgamation.reserve(current_batch_size * inputs[0].size());
            outputs_amalgamation.reserve(current_batch_size * outputs[0].size());
            if(!go_on_incorrects)
            {
                for(int i = buff_size_processed; i < buff_size_processed + current_batch_size; i++)
                {
                    for(int a = 0; a < inputs[i].size(); a++)
                        inputs_amalgamation.push_back(inputs[i][a]);
                    for(int a = 0; a < outputs[i].size(); a++)
                        outputs_amalgamation.push_back(outputs[i][a]);
                }
            }
            else
            {
                for(int i = buff_size_processed; i < buff_size_processed + current_batch_size; i++)
                {
                    for(int a = 0; a < inputs[incorrectInputPredictions[i]].size(); a++)
                        inputs_amalgamation.push_back(inputs[incorrectInputPredictions[i]][a]);
                    for(int a = 0; a < outputs[incorrectInputPredictions[i]].size(); a++)
                        outputs_amalgamation.push_back(outputs[incorrectInputPredictions[i]][a]);
                }
            }

            if(current_batch_size == butch_size && !setupped_for_butch_size)
            {
                nn.InitGPUBuffers(current_batch_size);
                setupped_for_butch_size=true;
            }
            if(current_batch_size != butch_size)
            {
                setupped_for_butch_size=false;
                nn.InitGPUBuffers(current_batch_size);
            }

            nn.BackPropagateGPU(inputs_amalgamation.data(),outputs_amalgamation.data(),learningRate,current_batch_size);
            nn.ReadGPUOutput(current_batch_size);
            bool good = true;
            for (int a = 0; a < outputs_amalgamation.size(); a++)
            {
                float predictedOutput = nn.outputs[a];
                float error = predictedOutput - outputs_amalgamation[a];
                float accepted_diff = 0.5f;
                if(textConvertion[firstOutputColumn + a])
                    accepted_diff = 1.0f / stringValue[firstOutputColumn + a].size();

                if(abs(error) > accepted_diff)
                {
                    if(go_on_incorrects)
                        incorrectInputPredictionsBuffer.push_back(incorrectInputPredictions[buff_size_processed + a]);
                    else
                        incorrectInputPredictionsBuffer.push_back(buff_size_processed + a);

                    //qDebug () << "error " +QVariant(buff_size_processed + a).toString()  <<error << predictedOutput  << outputs_amalgamation[a];
                }
                else
                    tmpcost  += 1;
            }

            buff_size_processed+= current_batch_size;
        }
        if(!stopRequested && !go_on_incorrects)
            cost = tmpcost;

        if(go_on_incorrects)
            onlyErrorIterationCount++;
        //if(go_on_incorrects)
        //    cost += inputs.size() - incorrectInputPredictions.size();

        incorrectInputPredictions= incorrectInputPredictionsBuffer;
        incorrectInputPredictionsBuffer.clear();


    }
}


void PredictorWindow::closeEvent(QCloseEvent* event)
{
    ptrPredictorWindow = nullptr;
}

PredictorWindow::~PredictorWindow()
{
    ptrPredictorWindow = nullptr;
    delete ui;
}

void PredictorWindow::on_browse_pressed()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select csv file"),ui->saveNameLineEdit->text());

    if(file.size()>0)
        ui->saveNameLineEdit->setText(file);
}


void PredictorWindow::on_loadPushButton_pressed()
{
    int arch[] = {1,2,3};
    nn.Create(arch,3);
    nn.LoadFrom(ui->saveNameLineEdit->text().toStdString());


    ui->hiddenLayerCountSlider->setValue(nn.LayersAmount-2);
    ui->hiddenLayerSizeSlider->setValue(nn.Arch[nn.LayersAmount-2]);

    ui->columnsToIgnoreSlider->setValue(0);
    ui->inputCountSlider->setValue(nn.Arch[0] -1);
    ui->outputCountSlider->setValue(nn.Arch[0] + nn.Arch[nn.LayersAmount-1] -1);

}


void PredictorWindow::on_savePushButton_pressed()
{
    nn.SaveTo(ui->saveNameLineEdit->text().toStdString());
}

void PredictorWindow::processData()
{
    DatabaseConnection* dc = sourceWindow->dc;

    if(dc->data.tbldata.size() <=0)
        return;

    ui->stopPushButton->show();

    //create mapping for tokens, check what outputs will be needed to swapped back to text
    inputs.clear();
    outputs.clear();
    stringValue.clear();
    textConvertion.clear();

    //Create selected structure if needed
    firstInputColumn = ui->inputCountSlider->minimum();
    lastInputColumn = ui->inputCountSlider->value();
    firstOutputColumn = ui->outputCountSlider->minimum();
    lastOutputColumn = ui->outputCountSlider->value();
    inputSize = lastInputColumn - firstInputColumn + 1;
    outputSize = lastOutputColumn - firstOutputColumn + 1;

    if(inputSize!= nn.sizein || outputSize != nn.sizeout)
        on_recreatePushButton_pressed();


    if(firstOutputColumn> dc->data.tbldata.size())// just quit, learningis pointless
        return;

    if(lastOutputColumn > dc->data.tbldata.size()) // tolerable
        lastOutputColumn = dc->data.tbldata.size();



    stringValue.resize(dc->data.tbldata.size());

    textConvertion.resize(dc->data.tbldata.size());

    // create buffers
    inputs.resize(dc->data.tbldata[0].size());
    outputs.resize(dc->data.tbldata[0].size());
    for(int i=0;i < dc->data.tbldata[0].size(); i++)
    {
        inputs[i].resize(inputSize);
        outputs[i].resize(outputSize);
    }

    // pass column by column, get unique strings, assign values to them
    for(int a= firstInputColumn;a <= lastOutputColumn; a++)
    {
        if(stopRequested)
        {
            break;
        }
        int valueCount = 0;
        int textCount = 0;
        for(int i= 0;i < dc->data.tbldata[a].size(); i++)
        {
            if(stopRequested)
            {
                break;
            }
            // check types
            bool ok = false;
            float value = dc->data.tbldata[a][i].toFloat(&ok);
            if(ok)
                valueCount++;
            else
            {
                textCount++;
                stringValue[a][dc->data.tbldata[a][i].trimmed()] = 1;
            }
        }
        if(textCount>= valueCount)
        {
            textConvertion[a] = true;
            // check types again, but now also store all the floats
            for(int i= 0;i < dc->data.tbldata[a].size(); i++)
            {
                bool ok = false;
                float value = dc->data.tbldata[a][i].toFloat(&ok);
                if(ok)
                    stringValue[a][dc->data.tbldata[a][i].trimmed()] = 1;
            }

        }
        int i=0;
        float divisor = (float)(stringValue[a].size() - 1);
        if(divisor<=0)
            divisor = 1;
        for(auto x :  stringValue[a])
        {
            stringValue[a][x.first] = ((float)i) / divisor;

            qDebug() << x.first << stringValue[a][x.first];
            i++;
        }
    }



    // fill output/input buffers  for NN
    for(int a= firstInputColumn;a <= lastOutputColumn; a++)
    {
        if(stopRequested)
        {
            break;
        }
        for(int i= 0;i < dc->data.tbldata[a].size(); i++)
        {
            if(stopRequested)
            {
                break;
            }
            float value = 0;

            // check types
            if(!textConvertion[a])
            {
                value = dc->data.tbldata[a][i].toFloat();
            }
            else
            {
                value = stringValue[a][dc->data.tbldata[a][i].trimmed()];

            }

            if(a <= lastInputColumn)
            {
                inputs[i][a - firstInputColumn] = value;
            }
            else
            {
                outputs[i][a - firstOutputColumn] = value;
            }
        }
    }
    stopRequested = false;
    qDebug() << textConvertion;
}
void async_Training(PredictorWindow* src)
{
    src->train();
}

void PredictorWindow::train()
{
    processData();

    exectimer.setInterval(1);
    currentIteration = 0;
    iterationCount = ui->iteratorSpinBox->value();
    // setup done, run learning iterations
    int iter_count = iterationCount;
    first_iter = true;
    timer_training = true;



}
void PredictorWindow::on_trainPushButton_clicked()
{
    ui->stopPushButton->show();

    learningRate = ui->rateSpinBox->value();

    // QThread* thr = QThread::create(async_Training,this);
    // thr->start();
    train();

    //ui->stopPushButton->hide();
}


void PredictorWindow::on_predictPushButton_pressed()
{
    processData();
    // run current nn on all data


    DatabaseConnection* dc = sourceWindow->dc;

    int sizebuff = dc->data.tbldata.size();
    dc->data.tbldata.resize(sizebuff + nn.sizeout);

    std::vector<int> type_str_buff;
    type_str_buff.resize(11);
    type_str_buff[10] =1;

    for(int i=0;i < nn.sizeout;i++)
    {
        dc->data.headers.push_back("NN_Output_" + QVariant(i).toString());
        dc->data.tbldata[sizebuff + i].resize(dc->data.tbldata[0].size());
        dc->data.maxVarTypes.push_back(10);// QString
        dc->data.typecount.push_back(type_str_buff);
    }




    int buff_max_size = inputs.size();
    int buff_size_processed = 0;
    int buff_size_left = buff_max_size;

    int butch_size = 1000;

    bool setupped_for_butch_size = false;


    while (buff_size_left > 0)
    {
        int current_batch_size = 1;

        if(buff_size_left >= butch_size)
        {
            buff_size_left -= butch_size;
            current_batch_size = butch_size;
        }
        else
        {
            current_batch_size = buff_size_left;
            buff_size_left = 0;
        }
        std::vector<float> inputs_amalgamation;
        inputs_amalgamation.reserve(current_batch_size * inputs[0].size());
        for(int i = buff_size_processed; i < buff_size_processed + current_batch_size; i++)
        {
            for(int a = 0; a < inputs[i].size(); a++)
                inputs_amalgamation.push_back(inputs[i][a]);

        }
        if(current_batch_size == butch_size && !setupped_for_butch_size)
        {
            nn.InitGPUBuffers(current_batch_size);
            setupped_for_butch_size=true;
        }
        if(current_batch_size != butch_size)
        {
            setupped_for_butch_size=false;
            nn.InitGPUBuffers(current_batch_size);
        }

        nn.RunGPU(inputs_amalgamation.data(),current_batch_size);
        //nn.ReadGPUOutput(current_batch_size);
        int sizeid = 0;
        int i = 0;
        for (int a = 0; a < current_batch_size * nn.sizeout; a++)
        {
            if(!textConvertion[firstOutputColumn + sizeid])
                dc->data.tbldata[sizebuff + sizeid][buff_size_processed + i] = QVariant(nn.outputs[a]).toString();
            else
            {
                float val = nn.outputs[a];
                float mindiff = 1000000000.0f;
                QString lasttext = "TEXT_NOT_FOUND";
                for (auto x : stringValue[firstOutputColumn + sizeid])
                    if(abs(x.second - val) < mindiff)
                    {
                        mindiff = abs(x.second - val);
                        lasttext = x.first;
                    }
                dc->data.tbldata[sizebuff + sizeid][buff_size_processed + i] = lasttext;
            }

            sizeid++;
            if(sizeid>= nn.sizeout)
            {
                sizeid = 0;
                i++;
            }
        }

        buff_size_processed+= current_batch_size;
    }
    // setup done, run learning iterations
    // for(int i=0;i < inputs.size() ;i++)
    // {
    //     nn.Run(inputs[i].data());


    //     for(int a=0;a < nn.sizeout;a++)
    //     {
    //         if(!textConvertion[firstOutputColumn + a])
    //             dc->data.tbldata[sizebuff + a][i] = QVariant(nn.outputs[a]).toString();
    //         else
    //         {
    //             float val = nn.outputs[a];
    //             float mindiff = 1000000000.0f;
    //             QString lasttext = "TEXT_NOT_FOUND";
    //             for (auto x : stringValue[firstOutputColumn + a])
    //                 if(abs(x.second - val) < mindiff)
    //                 {
    //                     mindiff = abs(x.second - val);
    //                     lasttext = x.first;
    //                 }
    //             dc->data.tbldata[sizebuff + a][i] = lasttext;
    //         }
    //     }
    // }

    sourceWindow->UpdateTable();
}


void PredictorWindow::on_stopPushButton_pressed()
{
    timer_training = false;
    stopRequested = true;
    ui->stopPushButton->hide();
    exectimer.setInterval(16);
    nn.ReadGPUWeightsBiases();
}


void PredictorWindow::on_columnsToIgnoreSlider_valueChanged(int value)
{
    if( value<sourceWindow->dc->data.headers.size())
        ui->ignoreColumnsLabel->setText(QVariant(value).toString());
    else
        ui->ignoreColumnsLabel->setText("out of range!");
    ui->inputCountSlider->setMinimum(value);
    on_inputCountSlider_valueChanged(ui->inputCountSlider->value());

}


void PredictorWindow::on_inputCountSlider_valueChanged(int value)
{
    if(value<sourceWindow->dc->data.headers.size())
        ui->InputsLabel->setText(sourceWindow->dc->data.headers[ui->inputCountSlider->minimum()] + " - " +  sourceWindow->dc->data.headers[value]);
    else
        ui->InputsLabel->setText("out of range!");

    ui->outputCountSlider->setMinimum(value +1);

    on_outputCountSlider_valueChanged(ui->outputCountSlider->value());
}


void PredictorWindow::on_outputCountSlider_valueChanged(int value)
{
    if(value < sourceWindow->dc->data.headers.size())
        ui->outputsLabel->setText(sourceWindow->dc->data.headers[ui->outputCountSlider->minimum()] + " - " + sourceWindow->dc->data.headers[value]);
    else
        ui->outputsLabel->setText("out of range!");

}


void PredictorWindow::on_hiddenLayerCountSlider_valueChanged(int value)
{
    ui->hiddenLayerCountLabel->setText(QVariant(value).toString());

}


void PredictorWindow::on_hiddenLayerSizeSlider_valueChanged(int value)
{
    ui->hiddenLayerSizeLabel->setText(QVariant(value).toString());

}


void PredictorWindow::updateSourceData()
{
    int value = 0;
    if(sourceWindow->dc->data.headers.size()-1>=0)
        value  = sourceWindow->dc->data.headers.size()-1;
    ui->columnsToIgnoreSlider->setMaximum(value);
    ui->inputCountSlider->setMaximum(value);
    ui->outputCountSlider->setMaximum(value);

    on_columnsToIgnoreSlider_valueChanged(ui->columnsToIgnoreSlider->value());
    on_inputCountSlider_valueChanged(ui->inputCountSlider->value());
    on_outputCountSlider_valueChanged(ui->outputCountSlider->value());
}


void PredictorWindow::on_recreatePushButton_pressed()
{
    ui->iterationProgressBar->setValue(0);
    ui->costProgressBar->setValue(0);


    std::vector<int> arch;
    arch.push_back(ui->inputCountSlider->value() - ui->inputCountSlider->minimum() + 1 );
    for(int i=0;i < ui->hiddenLayerCountSlider->value();i++)
        arch.push_back(ui->hiddenLayerSizeSlider->value());
    arch.push_back(ui->outputCountSlider->value() - ui->outputCountSlider->minimum() + 1);

    nn.Create(arch.data(),arch.size());
    nn.Randomize();

    ui->totalNodesLabel->setText("Total NN nodes count: " + QVariant(nn.NodesAmount).toString());
    ui->totalWeightsLabel->setText("Total weights count: " + QVariant(nn.WeightsAmount).toString());




}

