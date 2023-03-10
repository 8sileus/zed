#ifndef ZED_NET_ASYNIO_H_
#define ZED_NET_ASYNIO_H_

#include "zed/coroutine/hint.hpp"
#include "zed/coroutine/task.hpp"
#include "zed/net/executor.h"
#include "zed/net/fd_event.h"
#include "zed/net/timer.h"

#include <netinet/in.h>

namespace zed {

namespace net {

    namespace asyn {

        namespace detail {

            struct AddEventAwaiter {
                FdEvent* m_fd_event {nullptr};
                int      m_events {0};

                AddEventAwaiter(FdEvent* fd_event, int events) noexcept
                    : m_fd_event(fd_event), m_events(events)
                {
                }

                constexpr bool await_ready() noexcept { return false; }

                void await_suspend(std::coroutine_handle<> who_call_me)
                {
                    m_fd_event->setHandle(who_call_me);
                    m_fd_event->addEvents(m_events);
                }

                void await_resume()
                {
                    m_fd_event->delEvents(m_events);
                    m_fd_event->setHandle(nullptr);
                }
            };

            struct TimerEventAwaiter {
                FdEvent*        m_fd_event {nullptr};
                int             m_events {0};
                bool            is_timeout {false};
                TimerEvent::Ptr m_timer_event {};

                TimerEventAwaiter(FdEvent* fd_event, int events) noexcept
                    : m_fd_event(fd_event), m_events(events)
                {
                }
                constexpr bool await_ready() noexcept { return false; }

                void await_suspend(std::coroutine_handle<> who_call_me)
                {
                    auto task = [this, who_call_me]() {
                        this->is_timeout = true;
                        who_call_me.resume();
                    };
                    m_timer_event
                        = std::make_shared<TimerEvent>(Executor::GetEpollTimeout(), false, task);
                    Executor::GetCurrentExecutor()->getTimer()->addTimerEvent(m_timer_event);
                    m_fd_event->setHandle(who_call_me);
                    m_fd_event->addEvents(m_events);
                }

                bool await_resume()
                {
                    Executor::GetCurrentExecutor()->getTimer()->delTimerEvent(m_timer_event);
                    m_fd_event->delEvents(m_events);
                    m_fd_event->setHandle(nullptr);
                    return is_timeout;
                }
            };

        } // namespace detail

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Read(int fd, void* buf, size_t count);

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Write(int fd, const void* buf, size_t count);

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t>
        Send(int fd, const void* buf, size_t count, int flags);

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Recv(int fd, void* buf, size_t count, int flags);

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Connect(int fd, const sockaddr* addr, socklen_t addrlen);

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags);

        [[CO_AWAIT_HINT]] coroutine::Task<int> Close(int fd);

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t>
        Readv(int fd, const struct iovec* vec, int count);

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t>
        Writev(int fd, const struct iovec* vec, int count);

        // [[CO_AWAIT_HINT]] coroutine::Task<int> Shutdown(int sockfd, int flag);
    } // namespace asyn

} // namespace net

} // namespace zed

#endif // ZED_NET_ASYNIO_H_