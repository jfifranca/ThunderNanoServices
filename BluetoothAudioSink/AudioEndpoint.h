/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Module.h"
#include "IAudioCodec.h"
#include "IAudioContentProtection.h"
#include "Tracing.h"

#include <interfaces/IBluetoothAudio.h>

namespace WPEFramework {

namespace A2DP {

    class AudioEndpoint {
    private:
        static constexpr uint16_t CommandTimeout = 1000;

    public:
        AudioEndpoint() = delete;
        AudioEndpoint& operator=(const AudioEndpoint&) = delete;

        AudioEndpoint(Bluetooth::AVDTPSocket& socket, const Bluetooth::AVDTPProfile::StreamEndPoint& sep, uint8_t sourceSEID)
            : _acpSeid(sep.SEID())
            , _intSeid(sourceSEID)
            , _socket(socket)
            , _command()
            , _codec(nullptr)
            , _cp(nullptr)
            , _cpEnabled(false)
        {
            ParseCapabilities(sep);
        }
        ~AudioEndpoint()
        {
            delete _cp;
            delete _codec;
        }

    public:
        uint8_t SEID() const
        {
            return (_acpSeid);
        }
        IAudioCodec* Codec() const
        {
            return (_codec);
        }
        IAudioContentProtection* ContentProtection() const
        {
            return (_cp);
        }
        void Configuration(IAudioCodec::StreamFormat& format, string& settings, bool& cpEnabled)
        {
            ASSERT(_codec != nullptr);
            _codec->Configuration(format, settings);
            cpEnabled = _cpEnabled;
        }

        uint32_t Configure(const IAudioCodec::StreamFormat& format, const string& settings, const bool enableCP = false);

        uint32_t Open()
        {
            return (CmdOpen());
        }
        uint32_t Close()
        {
            return (CmdClose());
        }
        uint32_t Start()
        {
            return (CmdStart());
        }
        uint32_t Stop()
        {
            return (CmdSuspend());
        }

    private:
        uint32_t CmdSetConfiguration(Bluetooth::AVDTPSocket::SEPConfiguration& config)
        {
            uint32_t result = Core::ERROR_NONE;
            _command.SetConfiguration(SEID(), _intSeid, config);
            if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {

                // Try to use "reconfigure" signal instead...
                _command.Reconfigure(SEID(), _intSeid, config);
                if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                    result = Core::ERROR_ASYNC_FAILED;
                    TRACE(Trace::Error, (_T("Failed to set endpoint configuration, SEID 0x%02x"), SEID()));
                }
            }
            return (result);
        }
        uint32_t CmdGetConfiguration()
        {
            uint32_t result = Core::ERROR_NONE;
            _command.GetConfiguration(SEID());
            if ((_socket.Exchange(CommandTimeout, _command, _command) == Core::ERROR_NONE) && (_command.Result().Status() == Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                _command.Result().ReadConfiguration([this](const uint8_t category, const Bluetooth::Buffer& /* data */) {
                    switch(category) {
                    case Bluetooth::AVDTPProfile::StreamEndPoint::ServiceCapabilities::CONTENT_PROTECTION:
                        // TODO
                        break;
                    case Bluetooth::AVDTPProfile::StreamEndPoint::ServiceCapabilities::MEDIA_CODEC:
                        // TODO
                        break;
                    }
                });
            } else {
                result = Core::ERROR_ASYNC_FAILED;
                TRACE(Trace::Error, (_T("Failed to read endpoint configuration, SEID 0x%02x"), SEID()));
            }
            return (result);
        }
        uint32_t CmdOpen()
        {
            uint32_t result = Core::ERROR_NONE;
            _command.Open(SEID());
            if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                result = Core::ERROR_ASYNC_FAILED;
                TRACE(Trace::Error, (_T("Failed to open endpoint, SEID 0x%02x"), SEID()));
            }
            return (result);
        }
        uint32_t CmdClose()
        {
            uint32_t result = Core::ERROR_NONE;
            _command.Close(SEID());
            if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                result = Core::ERROR_ASYNC_FAILED;
                TRACE(Trace::Error, (_T("Failed to close endpoint, SEID 0x%02x"), SEID()));
            }
            return (result);
        }
        uint32_t CmdStart()
        {
            uint32_t result = Core::ERROR_NONE;
            _command.Start(SEID());
            if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                result = Core::ERROR_ASYNC_FAILED;
                TRACE(Trace::Error, (_T("Failed to start endpoint, SEID 0x%02x"), SEID()));
            }
            return (result);
        }
        uint32_t CmdSuspend()
        {
            uint32_t result = Core::ERROR_NONE;
            _command.Suspend(SEID());
            if ((_socket.Exchange(CommandTimeout, _command, _command) != Core::ERROR_NONE) || (_command.Result().Status() != Bluetooth::AVDTPSocket::Command::Signal::SUCCESS)) {
                result = Core::ERROR_ASYNC_FAILED;
                TRACE(Trace::Error, (_T("Failed to suspend endpoint, SEID 0x%02x"), SEID()));
            }
            return (result);
        }

    private:
        void ParseCapabilities(const Bluetooth::AVDTPProfile::StreamEndPoint& sep);

    private:
        uint8_t _acpSeid;
        uint8_t _intSeid;
        Bluetooth::AVDTPSocket& _socket;
        Bluetooth::AVDTPSocket::Command _command;
        IAudioCodec* _codec;
        IAudioContentProtection* _cp;
        bool _cpEnabled;
    }; // class AudioEndpoint

} // namespace A2DP

}
