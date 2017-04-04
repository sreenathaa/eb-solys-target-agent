/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef TARGETAGENTTSPROVIDERINTERFACE_H_
#define TARGETAGENTTSPROVIDERINTERFACE_H_

#ifdef __cplusplus

namespace TargetAgent {
namespace PluginInterface {

/**
 * Unified mechanism for timestamp computation
 */
class CTimestampProvider {
public:
    /**
     * Create a timestamp according to the configuration. Function intend is to provide a unified Ts basis for all plugins
     * @param none
     * @see -
     * @return true:  startup phase of the plugin was successfully trigger
     *      false:   error occurred during the startup phase
     */
    virtual unsigned long long createTimestamp(void) const = 0;

    CTimestampProvider() {
    }
    CTimestampProvider(const CTimestampProvider& other);
    CTimestampProvider& operator=(const CTimestampProvider& other);
    virtual ~CTimestampProvider() {
    }
};

}
}
#else
typedef
struct CTimestampProvider CTimestampProvider;
#endif

#endif /* TARGETAGENTTSPROVIDERINTERFACE_H_ */
