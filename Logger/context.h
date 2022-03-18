#pragma once

#include <functional>
#include <queue>
#include <boost/thread.hpp>


class Context
{
    using Task = std::function<void()>;

public:
    class Work;
    class Strand;

    Context()
        : m_isWorking(false)
    {
    }

    void post(Task task)
    {
        boost::unique_lock<boost::shared_mutex> tasksLock(m_tasksChange);
        m_tasks.push(task);
    }

    void poll()
    {
        Task task;
        {
            boost::upgrade_lock<boost::shared_mutex> sharedTasksLock(m_tasksChange);
            if (m_tasks.empty())
                return;

            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueTasksLock(sharedTasksLock);
            task = m_tasks.front();
            m_tasks.pop();

            /* {
                 boost::shared_lock<boost::shared_mutex> tasksLock(m_tasksChange);
                 if (m_tasks.empty())
                     return;
             }

             {
                 boost::unique_lock<boost::shared_mutex> tasksLock(m_tasksChange);
                 task = m_tasks.front();
                 m_tasks.pop();
             }*/
        }
        task();
    }

    void run()
    {
        bool empty = false;
        do
        {
            {
                boost::shared_lock<boost::shared_mutex> tasksLock(m_tasksChange);
                empty = m_tasks.empty();
            }
            poll();
        } while (!empty || m_isWorking);
    }

private:
    boost::shared_mutex m_tasksChange;

    std::queue<Task> m_tasks;
    bool m_isWorking;
};

class Context::Work
{
public:
    Work(Context& context)
        : m_context(context)
    {
        m_context.m_isWorking = true;
    }
    ~Work()
    {
        m_context.m_isWorking = false;
    }
private:
    Context& m_context;
};

class Context::Strand
{
public:
    Strand(Context& context)
        : m_context(context)
        , m_task(nullptr)
        , thread([this]() { Run(); })
    {
    }

    void post(Task task)
    {
        m_tasks.push(task);
        // m_context.post(task);

    }

private:
    void Run()
    {
        while (true)
        {
            if (!m_tasks.empty())
            {
                auto& task = m_tasks.front();
                bool& isProcessed = m_context.post(task);
                while (!isProcessed)
                {
                }
            }
        }
    }

    std::queue<Task> m_tasks;
    Context& m_context;
    boost::thread thread;

    Task* m_task;
};