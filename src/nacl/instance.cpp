#include <cstdio>
#include <errno.h>
#include <sys/mount.h>

#include "nacl_io/nacl_io.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/module.h"

#include "instance.h"

using namespace CTox;

namespace CTox {
    static const std::string a_type_configure = "CONFIGURE";
    static const std::string a_type_connect = "CONNECT";
};

CToxInstance::CToxInstance(PP_Instance instance) : pp::Instance(instance),
                                                   pp_core_(pp::Module::Get()->core()),
                                                   callback_factory_(this),
                                                   worker_thread_(this) { }

CToxInstance::~CToxInstance() {
    umount( FS_PATH);
    nacl_io_uninit();
}

bool CToxInstance::Init(uint32_t argc,
                        const char *argn[],
                        const char *argv[]) {

    int naclError = nacl_io_init_ppapi(pp_instance(), pp::Module::Get()->get_browser_interface());

    if (naclError != PP_OK) {
        fprintf(stderr, "Failed to initialize nacl_io");
    }

    int mountError = mount("", FS_PATH, "html5fs", 0, "type=PERSISTENT,expected_size=1048576");

    if (mountError != 0) {
        fprintf(stderr, "Failed to mount html5fs");
    }

    return worker_thread_.Start();
}


/**
 * Handle as message from JavaScript on the worker thread.
 *
 * @param[in] message The message to parse and handle.
 */
void CToxInstance::HandleMessage(const pp::Var &var_message) {
    fprintf(stderr, "New message\n");

    if (!var_message.is_dictionary()) {
        fprintf(stderr, "Message handler error: Argument is not a dictionary\n");
        return;
    }

    pp::VarDictionary message(var_message);

    // Get the message ID
    int id = message.Get("id").AsInt();

    // Get the message action type
    std::string actionType = message.Get("type").AsString();

    fprintf(stderr, "Handling message ID %d action type %s\n", id, actionType.c_str());

    if (actionType == a_type_configure) {
        // Configure action
        worker_thread_.message_loop().PostWork(callback_factory_.NewCallback(&CToxInstance::Configure));
    } else if (actionType == a_type_connect) {
        // Connect action
    } else {
        fprintf(stderr, "Message handler error: Invalid action type %s\n", actionType.c_str());
    }
}

void CToxInstance::Configure(int32_t /*result*/) {
    fprintf(stderr, "Configuring\n");

    tox_core_instance_ = CToxTox::Core::Create();

    pp::VarDictionary data;
    data.Set(pp::Var("tox_id"), "123456789");

    SendResponse(1, std::string("foo"), data);
}

void CToxInstance::SendResponse(const int request_id,
                                const std::string &status,
                                const pp::VarDictionary &data) {
    pp::CompletionCallback callback = callback_factory_.NewCallback(&CToxInstance::SendResponseAsJSONObject,
                                                                    request_id,
                                                                    status,
                                                                    data);

    pp_core_->CallOnMainThread(0, callback);
}

void CToxInstance::SendResponseAsJSONObject(int32_t result,
                                            const int request_id,
                                            const std::string &status,
                                            const pp::VarDictionary &data) {
    fprintf(stderr, "Create JSON response\n");

    // Create the response dictionary
    pp::VarDictionary response;

    // Set the response values
    response.Set(pp::Var("id"), pp::Var(request_id));
    response.Set(pp::Var("status"), pp::Var(status));
    response.Set(pp::Var("data"), data);

    // Post the shebang
    PostMessage(response);

    fprintf(stderr, "Response sent\n");
}

//std::shared_ptr <CToxTox::Core> &CToxInstance::ToxCoreInstance() {
//    return tox_core_instance_;
//}