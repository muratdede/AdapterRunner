#ifndef BATHREAD_H
#define BATHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QVector>

class BAThread : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning WRITE setIsRunning NOTIFY isRunningChanged)
public:
    BAThread();
    /**
     * @brief ~BAThread
     *  if deletelater used, stop() must be called before
     *  if delete used, calls stop inside
     */
    virtual ~BAThread();

    void start();
    /**
     * @brief stop
     *  object affinity changes to caller thread
     */
    void stop();

    bool isRunning() const; // status check

signals:
    void isRunningChanged(bool isRunning);

protected:
    /**
     * @brief getShouldStop
     *  use this for checking if mainloop should exit or continue to run,
     *      like: while(!getShouldStop()) ...
     * @return
     */
    bool getShouldStop() const;

protected:
    //Args format must be Q_ARG(type_of_Variable, variable)
    template<typename ...Args>
    bool makeMethodRunOnThisThread(const char *methodName, Args... args){
        static_assert(sizeof...(args) <= MAX_ARGUMENT_COUNT, "You can't have more than 10 arguments!");

        if (thread() == QThread::currentThread()) { // if method is called in thisthread context, just return true
            return true;
        } // else invoke method in thisthread context, return false

        QVector<QGenericArgument> tGenericArgumentLis(MAX_ARGUMENT_COUNT, QGenericArgument(nullptr));
        const QVector<QGenericArgument> tTemplist = {args ...};

        for (int index = 0; index < tTemplist.size(); ++index) {
            tGenericArgumentLis[index] = tTemplist.at(index);
        }

        QMetaObject::invokeMethod(this, methodName,
                                  tGenericArgumentLis.at(0), tGenericArgumentLis.at(1), tGenericArgumentLis.at(2),
                                  tGenericArgumentLis.at(3), tGenericArgumentLis.at(4), tGenericArgumentLis.at(5),
                                  tGenericArgumentLis.at(6), tGenericArgumentLis.at(7), tGenericArgumentLis.at(8),
                                  tGenericArgumentLis.at(9));

        return false;
    }

private slots:
    void setIsRunning(bool isRunning);
    void outerLoop();

private:
    /**
     * @brief mainLoop
     *  derived must implement this for mainLoop
     *
     *  example mainloop to use eventloop is as follows:
     *  ...
     *  #include <QAbstractEventDispatcher>
     *  ...
     *  while(!getShouldStop()) {
     *      QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
     *  }
     *  ...
     */
    virtual void mainLoop() = 0;
    /**
     * @brief exitThreadLoop
     *  stops eventloop if getshouldstop is used in mainloop
     *  also wakes up eventdispatcher if it is used in mainloop, too
     */
    void exitThreadLoop();

    QThread mThread;
    QThread *mStoppingThread = nullptr;
    bool mIsRunning;
    bool shouldStop;

private:
    const static int MAX_ARGUMENT_COUNT = 10;
};

#endif // BATHREAD_H
