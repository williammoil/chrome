#ifndef CTOX_MODULE_H
#define CTOX_MODULE_H

#include "ppapi/c/pp_instance.h"

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"

namespace CTox {

    class CToxModule : public pp::Module {
    public:
        CToxModule();

        virtual ~CToxModule();

        virtual pp::Instance *CreateInstance(PP_Instance instance);
    };

}

#endif
