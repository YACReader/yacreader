/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo
*<giuseppe.dangelo@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

#include "KDSignalThrottler.h"

namespace KDToolBox
{

KDGenericSignalThrottler::KDGenericSignalThrottler(Kind kind, EmissionPolicy emissionPolicy, QObject *parent)
    : QObject(parent)
    , m_timer(this)
    , m_kind(kind)
    , m_emissionPolicy(emissionPolicy)
    , m_hasPendingEmission(false)
{
    // For leading throttlers we use a repeated timer. This is in order
    // to catch the case where a signal is received by the throttler
    // just after it emitted a throttled/debounced signal. Even if leading,
    // it shouldn't re-emit immediately, as it would be too close to the previous one.
    // So we keep the timer running, and stop it later if it times out
    // with no intervening timeout() emitted by it.
    switch (m_emissionPolicy)
    {
    case EmissionPolicy::Leading:
        m_timer.setSingleShot(false);
        break;
    case EmissionPolicy::Trailing:
        m_timer.setSingleShot(true);
        break;
    }
    connect(&m_timer, &QTimer::timeout, this, &KDGenericSignalThrottler::maybeEmitTriggered);
}

KDGenericSignalThrottler::~KDGenericSignalThrottler()
{
    maybeEmitTriggered();
}

KDGenericSignalThrottler::Kind KDGenericSignalThrottler::kind() const
{
    return m_kind;
}

KDGenericSignalThrottler::EmissionPolicy KDGenericSignalThrottler::emissionPolicy() const
{
    return m_emissionPolicy;
}

int KDGenericSignalThrottler::timeout() const
{
    return m_timer.interval();
}

void KDGenericSignalThrottler::setTimeout(int timeout)
{
    if (m_timer.interval() == timeout)
        return;
    m_timer.setInterval(timeout);
    Q_EMIT timeoutChanged(timeout);
}

void KDGenericSignalThrottler::setTimeout(std::chrono::milliseconds timeout)
{
    setTimeout(int(timeout.count()));
}

Qt::TimerType KDGenericSignalThrottler::timerType() const
{
    return m_timer.timerType();
}

void KDGenericSignalThrottler::setTimerType(Qt::TimerType timerType)
{
    if (m_timer.timerType() == timerType)
        return;
    m_timer.setTimerType(timerType);
    Q_EMIT timerTypeChanged(timerType);
}

void KDGenericSignalThrottler::throttle()
{
    m_hasPendingEmission = true;

    switch (m_emissionPolicy)
    {
    case EmissionPolicy::Leading:
        // Emit only if we haven't emitted already. We know if that's
        // the case by checking if the timer is running.
        if (!m_timer.isActive())
            emitTriggered();
        break;
    case EmissionPolicy::Trailing:
        break;
    }

    // The timer is started in all cases. If we got a signal,
    // and we're Leading, and we did emit because of that,
    // then we don't re-emit when the timer fires (unless we get ANOTHER
    // signal).
    switch (m_kind)
    {
    case Kind::Throttler:
        if (!m_timer.isActive())
            m_timer.start(); // = actual start, not restart
        break;
    case Kind::Debouncer:
        m_timer.start(); // = restart
        break;
    }

    Q_ASSERT(m_timer.isActive());
}

void KDGenericSignalThrottler::maybeEmitTriggered()
{
    if (m_hasPendingEmission)
        emitTriggered();
    else
        m_timer.stop();
}

void KDGenericSignalThrottler::emitTriggered()
{
    Q_ASSERT(m_hasPendingEmission);
    m_hasPendingEmission = false;
    Q_EMIT triggered();
}

// Convenience

KDSignalThrottler::KDSignalThrottler(QObject *parent)
    : KDGenericSignalThrottler(Kind::Throttler, EmissionPolicy::Trailing, parent)
{
}

KDSignalThrottler::~KDSignalThrottler() = default;

KDSignalLeadingThrottler::KDSignalLeadingThrottler(QObject *parent)
    : KDGenericSignalThrottler(Kind::Throttler, EmissionPolicy::Leading, parent)
{
}

KDSignalLeadingThrottler::~KDSignalLeadingThrottler() = default;

KDSignalDebouncer::KDSignalDebouncer(QObject *parent)
    : KDGenericSignalThrottler(Kind::Debouncer, EmissionPolicy::Trailing, parent)
{
}

KDSignalDebouncer::~KDSignalDebouncer() = default;

KDSignalLeadingDebouncer::KDSignalLeadingDebouncer(QObject *parent)
    : KDGenericSignalThrottler(Kind::Debouncer, EmissionPolicy::Leading, parent)
{
}

KDSignalLeadingDebouncer::~KDSignalLeadingDebouncer() = default;

} // namespace KDToolBox
