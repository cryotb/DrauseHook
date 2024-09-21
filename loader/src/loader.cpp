#include "inc/include.h"

Loader* gLoader = nullptr;

void Loader::start(int argc, char* argv[])
{
#if defined(R5I_PROD)
  //msg("aes-key: '%s'", STRINGIFIED_DEF(_LOADER_COMPILATION_AES_KEY));
  //msg("email: '%s'", STRINGIFIED_DEF(_LOADER_USER_EMAIL));
  //msg("token: '%s'", STRINGIFIED_DEF(_LOADER_USER_ACCESS_TOKEN));
  //msg("ts: '%s'", STRINGIFIED_DEF(_BUILD_TIMESTAMP));

  if( std::string(STRINGIFIED_DEF(_LOADER_USER_EMAIL)).empty() || std::string(STRINGIFIED_DEF(_LOADER_USER_ACCESS_TOKEN)).empty() )
  {
    msg("internal error: AEFC");
    return;
  }

  if( std::string(STRINGIFIED_DEF(_LOADER_COMPILATION_AES_KEY)).empty() || std::string(STRINGIFIED_DEF(_BUILD_TIMESTAMP)).empty() )
  {
    msg("internal error: AEFD");
    return;
  }

  if(!setup_netsdk())
  {
    msg("networking error: 10AE");
    return;
  }

  if(!this->authenticate())
  {
    msg("failed authentication. please check your credentials!");
    std::string reason = "N/A";
    switch(gNetSsnContext->value_authentication_result)
    {
      case proto::AUTH_STATUS_INVALID_CREDENTIALS: reason ="invalid credentials"; break;
      case proto::AUTH_STATUS_USER_BANNED: reason = "you currently have an active ban on record (1)!"; break;
      case proto::AUTH_STATUS_HARDWARE_BANNED: reason = "you currently have an active ban on record (2)!"; break;
      case proto::AUTH_STATUS_VM_CHECKS_FAILED: reason = "your hardware is not genuine. please contact support for more information."; break;
      case proto::AUTH_STATUS_COOLDOWN: reason = "slow down! because of too many failed attempts, you will have to wait a few minutes before trying again."; break;
      case proto::AUTH_STATUS_CLIENT_SIDE_ERROR: reason = "an unexpected error has occurred during authentication attempt on the client side."; break;
      case proto::AUTH_STATUS_INTERNAL_ERROR: reason = "an unexpected error has occurred during authentication attempt internally."; break;
    }
    msg("failure reason: %s", reason.c_str());
    msg("do not hesistate to contact support by sending an email to support@cryot.de, should you believe this is an error, or require support in general.");
    return;
  }
#endif

  //

     /*
   * PRE-LAUNCH PROCEDURES
   */
  gctx = new Context();
  memset(&gctx->m_lctx, 0, sizeof(gctx->m_lctx));

#if !defined(R5I_PROD)
  if (argc > 1)
  {
    std::vector<std::string> args;

    for (int i = 1; i < argc; i++)
    {
      args.push_back(std::string(argv[i]));
    }

    for (const auto &arg : args)
      parse_argument(arg);
  }
#else
  gctx->m_opts.use_unsafe_memory = true;
#endif

  if(!tools::is_running_as_root())
  {
    safeLog_msg("!! you must run this as root !!");
    return;
  }

  if(tools::proc_name2id(TARGET_NAME) != 0 && (!gctx->m_opts.dump_game_image && !gctx->m_opts.debug_sigs))
  {
    safeLog_msg("!! you must run this before the game launches !!");
    return;
  }

#if !defined(R5I_PROD)
 std::string image_path = _XS("/mnt/dev-r5i/r5_internal_proton_llvm/patched_r5.dll");

  if(gctx->m_opts.use_custom_payload_path)
  {
    safeLog_msg("!! overriding image path with custom option: %s.", gctx->m_opts.custom_payload_path.c_str());
    image_path = gctx->m_opts.custom_payload_path;
  }

  std::vector<u8> image = tools::aob_read(image_path);
  if (image.empty())
  {
    safeLog_msg("failed to read default image.");
    return;
  }
#else
  //tools::hex_dump( vec_game_dump.data(), 0x1000 );

  if(!gNetSDK->request_dlc(0x6523))
  {
    msg("networking error: 10AF");
    return;
  }
  
  while(!gNetSsnContext->received_request_dlc_response)
  {
    _mm_pause();
    std::this_thread::sleep_for(100ms);
  }

  if(!gNetSsnContext->will_server_send_requested_dlc)
  {
    msg("networking error: 10AD");
    return;
  }

  msg("confirmed!");

  while(!gNetSsnContext->received_dlc)
  {
    _mm_pause();
    std::this_thread::sleep_for(100ms);
  }

  std::vector<u8> image = gNetSsnContext->vec_dlc_contents;

  gctx->m_opts.analysis_from_dump = false;
#endif

  if (!mapper::fetch_image(image))
  {
    safeLog_msg("failed to fetch image.");
    return;
  }

  if (!mapper::resolve_linux_crt())
  {
    safeLog_msg("failed to resolve LINUX CRT runtime.");
    return;
  }

  if (!mapper::setup_pre())
  {
    safeLog_msg("failed to mapper setup PRE stage.");
    return;
  }

  /*
   * POST-LAUNCH PROCEDURES
   */
  safeLog_msg("waiting for the game...");
  msg("waiting for the game...");

  int pid = 0;
  while (!(pid = tools::proc_name2id(TARGET_NAME)))
  { /* ... */
  }

  safeLog_msg("process ID of the target is (%i)", pid);

  gmm = new MemoryManager(pid);

  if (!mapper::setup(pid))
  {
    safeLog_msg("failed setup!");
    return;
  }

  if (!mapper::load(image))
  {
    safeLog_msg("failed load!");
    return;
  }
  
  safeLog_msg("successfully load!");

  if (gctx->m_opts.payload_debug)
  {
    do
    {
      auto ctx = gmm->tread<LoaderContext>(gctx->m_ctx_base);

      if (!ctx.inited)
        break;

      usleep(500000);
    } while (true);
  }

  delete gmm;
  delete gctx;
}

bool Loader::setup_netsdk()
{
  msg("connecting... (1/7)");

  gNetSDK = new NetSDK("192.168.1.109", 10000);
  if (!gNetSDK->start())
  {
    msg("failed to establish connection with www.cryot.de.");
    return false;
  }

  msg("connecting... (2/7)");

  if (!gNetSDK->handshake())
  {
    msg("failed to establish connection with www.cryot.de (2).");
    return false;
  }

  msg("connecting... (3/7)");

  if (!gNetSDK->wait_for_activation())
  {
    msg("failed to establish connection with www.cryot.de (3).");
    return false;
  }

  msg("connecting... (4/7)");

  if (!gNetSDK->start_pump())
  {
    msg("failed to establish connection with www.cryot.de (4).");
    return false;
  }

  msg("connecting... (5/7)");

  if(!gNetSDK->ping())
  {
    msg("failed to establish connection with www.cryot.de (5).");
    return false;
  }

  msg("connecting... (6/7)");

  while(!gNetSsnContext->received_ping)
  {
    _mm_pause();
  }

  if(!gNetSsnContext->has_server_acknowledged_ping)
  {
    msg("failed to establish connection with www.cryot.de (6).");
    return false;
  }

  msg("connecting... (7/7)");

  return true;
}

bool Loader::authenticate()
{
  std::string input_email = STRINGIFIED_DEF(_LOADER_USER_EMAIL), input_password = STRINGIFIED_DEF(_LOADER_USER_ACCESS_TOKEN);

  if(input_email.empty() || input_password.empty())
  {
    msg("please fix those empty input fields.");
    return false;
  }

  if(!gNetSDK->authenticate(input_email, input_password))
  {
    msg("failed authentication due to networking error (1).");
    return false;
  }

  while(!gNetSsnContext->received_authentication_result)
  {
    _mm_pause();
  }

  return (gNetSsnContext->value_authentication_result == proto::AUTH_STATUS_SUCCESS);
}

std::string Loader::get_argument_param(const std::string &str)
{
  std::string delimiter = "=";
  std::size_t pos = str.find(delimiter);
  if (pos != std::string::npos)
  {
    // Extract the path by removing the prefix and '=' character
    std::string arg1 = str.substr(pos + delimiter.length());
    return arg1;
  }

  return "";
}

void Loader::parse_argument(const std::string &str)
{
#if !defined(R5I_PROD)
  if (str == "--debug-payload")
  {
    safeLog_msg("!! enabled payload debug mode !!");
    gctx->m_opts.payload_debug = true;
  }

  if (str == "--debug-sigs")
  {
    safeLog_msg("!! enable debug_SIGS mode !!");
    gctx->m_opts.debug_sigs = true;
  }

  if (str == "--dump-game-image")
  {
    safeLog_msg("!! enable dump_GAME_IMAGE mode !!");
    gctx->m_opts.dump_game_image = true;
  }

  if (str.find("--analysis-from-dump") != std::string::npos)
  {
    auto path = get_argument_param(str);
    if(!path.empty())
    {
      safeLog_msg("!! performing analysis on dump file at %s", path.c_str());
      gctx->m_opts.analysis_from_dump = true;
      gctx->m_opts.analysis_from_dump_path = path;
    }
  }

  if(str.find("--custom-payload-path") != std::string::npos)
  {
    auto path = get_argument_param(str);
    if(!path.empty())
    {
      safeLog_msg("!! using custom payload path, specified as %s", path.c_str());
      gctx->m_opts.use_custom_payload_path = true;
      gctx->m_opts.custom_payload_path = path;
    }
  }

  if (str.find("--use-unsafe-memory") != std::string::npos)
  {
    safeLog_msg("!! using unsafe memory for mapping the payload, this is UNSAFE as fuck.");
    gctx->m_opts.use_unsafe_memory = true;
  }
#endif
}
