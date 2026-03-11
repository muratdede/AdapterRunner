#include "src/core/bathread.h"

#include <QDebug>
#include <QAbstractEventDispatcher>

BAThread::BAThread()
    : QObject(nullptr)
    , mIsRunning(false)
    , shouldStop(false)
{

}

BAThread::~BAThread()
{
    stop();
}

void BAThread::start() {
    if(mIsRunning) {
        return;
    }

    shouldStop = false;

    moveToThread(&mThread);
    connect(&mThread, &QThread::started, this, &BAThread::outerLoop);
    mThread.start();
}

void BAThread::exitThreadLoop()
{
    shouldStop = true;
    if(mThread.eventDispatcher())
        mThread.eventDispatcher()->wakeUp();
}

void BAThread::stop() {
    if(!mIsRunning){
        return;
    }

    mStoppingThread = QThread::currentThread();
    exitThreadLoop();

    mThread.wait();
}

bool BAThread::isRunning() const
{
    return mIsRunning;
}

void BAThread::setIsRunning(bool isRunning)
{
    if (mIsRunning == isRunning)
        return;

    mIsRunning = isRunning;
    emit isRunningChanged(mIsRunning);
}

void BAThread::outerLoop() {
    setIsRunning(true);

    mainLoop();

    mThread.quit();
    disconnect(&mThread, &QThread::started, this, &BAThread::outerLoop);

    setIsRunning(false);
    if(mStoppingThread != nullptr) {
        moveToThread(mStoppingThread);
        mStoppingThread = nullptr;
    }
}

bool BAThread::getShouldStop() const
{
    return shouldStop;
}
