#include "databasetask.h"

DatabaseTask::DatabaseTask(QObject *parent)
    : QObject{parent}
{}

void DatabaseTask::exec()
{

}

/*
maybe gonna implement multithreaded query loading through task system, idono
*/
