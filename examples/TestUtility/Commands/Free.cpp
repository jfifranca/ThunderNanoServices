/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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
 
#include "../CommandCore/TestCommandBase.h"
#include "../CommandCore/TestCommandController.h"
#include "MemoryAllocation.h"

namespace WPEFramework {

class Free : public TestCommandBase {
public:
    Free(const Free&) = delete;
    Free& operator=(const Free&) = delete;

public:
    using Parameter = JsonData::TestUtility::InputInfo;

    Free()
        : TestCommandBase(TestCommandBase::DescriptionBuilder("Releases previously allocated memory"),
              TestCommandBase::SignatureBuilder("memory", JsonData::TestUtility::TypeType::NUMBER, "memory statistics in KB"))
        , _memoryAdmin(MemoryAllocation::Instance())
    {
        TestCore::TestCommandController::Instance().Announce(this);
    }

    ~Free() override
    {
        TestCore::TestCommandController::Instance().Revoke(this);
    }

public:
    // ICommand methods
    string Execute(const string& params VARIABLE_IS_NOT_USED) final
    {
        bool status = _memoryAdmin.Free();
        return (status == true ? _memoryAdmin.CreateResponse() : EMPTY_STRING);
    }

    string Name() const final
    {
        return _name;
    }

private:
    BEGIN_INTERFACE_MAP(Free)
    INTERFACE_ENTRY(Exchange::ITestUtility::ICommand)
    END_INTERFACE_MAP

private:
    MemoryAllocation& _memoryAdmin;
    const string _name = _T("Free");
};

static Free* _singleton(Core::Service<Free>::Create<Free>());

} // namespace WPEFramework
