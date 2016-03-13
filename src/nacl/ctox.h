#ifndef CTOX_H
#define CTOX_H

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/instance_handle.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "ppapi/utility/threading/simple_thread.h"

#include "nacl_io/nacl_io.h"

#include "event_listener.h"
#include "tox/core.h"

namespace CTox {
    class CToxInstance : public pp::Instance, public CToxEventListener {

    public:
        explicit CToxInstance(PP_Instance instance);

        virtual ~CToxInstance();

        virtual bool Init(uint32_t, const char *[], const char *[]);

        virtual void HandleMessage(const pp::Var &var_message);

    private:
        std::shared_ptr <CToxTox::Core> tox_core_instance_;

        pp::Core *pp_core_;
        pp::CompletionCallbackFactory <CToxInstance> callback_factory_;
        pp::SimpleThread worker_thread_;

        void Configure(int32_t);

        void SendResponse(const int request_id,
                          const std::string &status,
                          const pp::VarDictionary &data);

        void SendResponseAsJSONObject(int32_t result,
                                      const int request_id,
                                      const std::string &status,
                                      const pp::VarDictionary &data);
    };
}

class CToxModule : public pp::Module {

public:
    CToxModule();

    virtual ~CToxModule();

    virtual pp::Instance *CreateInstance(PP_Instance instance);

};

namespace pp {
    Module *CreateModule() {
        return new CToxModule();
    }
}

#endif