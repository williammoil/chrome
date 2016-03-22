#include "module.h"
#include "instance.h"

namespace CTox {

    CToxModule::CToxModule() : pp::Module() { }

    CToxModule::~CToxModule() { }

    pp::Instance *CToxModule::CreateInstance(PP_Instance instance) {
        static bool created = false;
        if (!created) {
            created = true;
            return new CToxInstance(instance);
        }
        return NULL;
    }

}

namespace pp {

    Module *CreateModule() {
        static bool created = false;
        if (!created) {
            created = true;
            return new CTox::CToxModule();
        }
        return NULL;
    }

}