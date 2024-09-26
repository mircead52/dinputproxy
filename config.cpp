
#include "stdafx.h"
#include "config.h"

#if useglib
#include "glib.h"
#endif

#include "ini.h"

#define LogInfo(...)


#define ERRFREE(X) if(X) { g_error_free(X); (X) = NULL; }
#define GFREE(X) if (X) { g_free(X); (X) = NULL; }

#define GROUP_DLL_PATH "DInput8Path"
#define GROUP_OVERRIDE "Override"
#define GROUP_BLACKLIST "Blacklist"

struct proxyconfig configx = { 0 };

#if useglib
void read_conf(void)
{
  GKeyFile* conf_file = g_key_file_new();
  GError *error = NULL;
    gchar* parsed_value = NULL;

  do {

    if (FALSE == g_key_file_load_from_file(conf_file, MODULE_NAME".ini", G_KEY_FILE_NONE, &error))
      {
          ERRFREE(error);
      break;
    }

    parsed_value = g_key_file_get_value(conf_file, GROUP_DLL_PATH, "Path", &error);
      if (parsed_value == NULL)
      {
      ERRFREE(error);
    }
    else
    {
      configx.dllpath = parsed_value;
      LogInfo("config path %s\r\n", configx.dllpath);
    }

    parsed_value = g_key_file_get_value(conf_file, GROUP_OVERRIDE, "MaskXONEController", &error);
      if (parsed_value == NULL)
      {
      ERRFREE(error);
      configx.mask_xone_ctrl = false;
    }
    else
    {
      configx.mask_xone_ctrl = (0 == g_ascii_strncasecmp(parsed_value,"yes",10) || (0 == g_ascii_strncasecmp(parsed_value, "1",10))) ? true : false;
      GFREE(parsed_value);
    }
    LogInfo("config xone mask %d\r\n", configx.mask_xone_ctrl);
    
    for(int i = 0; i < 20; i++)
    {
      guint64 tmp;
      char local[20];
      g_snprintf(local, sizeof(local), "AllowClass%u", i);
      parsed_value = g_key_file_get_value(conf_file, GROUP_OVERRIDE, local, &error);
        if (parsed_value == NULL)
        {
        ERRFREE(error);
        continue;
      }
      else
      {
        tmp = g_ascii_strtoull(parsed_value, 0, 0);
        configx.allowclass[(unsigned int)tmp] = true;
        GFREE(parsed_value);
      }
      LogInfo("config allow class %x\r\n", tmp);
    }
    
    for(int i = 0; i < 20; i++)
    {
      char local[20];
      g_snprintf(local, sizeof(local), "GUID%u", i);
      parsed_value = g_key_file_get_value(conf_file, GROUP_BLACKLIST, local, &error);
        if (parsed_value == NULL)
        {
        ERRFREE(error);
        continue;
      }
      else
      {
        configx.blacklist.insert(parsed_value);
        GFREE(parsed_value);
      }
      LogInfo("config allow class %x\r\n", tmp);
    }

  } while(0);
  
    g_key_file_free(conf_file);
}
#endif

static int handler(void* user, const char* section, const char* name,
    const char* value)
{
#define MATCH(s, n)  ((strcmp(section, s) == 0) && (strcmp(name, n) == 0))
#define MATCH2(s, n) ((strcmp(section, s) == 0) && (strncmp(name, n, sizeof(n) -1) == 0))

    if (MATCH(GROUP_DLL_PATH, "Path"))
    {
        configx.dllpath = _strdup(value);
        LogInfo("config path %s\r\n", configx.dllpath);
    }
    else if (MATCH(GROUP_DLL_PATH, "FindAllDlls"))
    {
    configx.find_all_dlls = (0 == strncmp(value, "yes", 10) || (0 == strncmp(value, "1", 10))) ? true : false;
    }
    else if (MATCH(GROUP_OVERRIDE, "MaskXONEController"))
    {
        configx.mask_xone_ctrl = (0 == strncmp(value, "yes", 10) || (0 == strncmp(value, "1", 10))) ? true : false;
    }
    else if (MATCH2(GROUP_OVERRIDE, "AllowClass"))
    {
        unsigned long tmp = strtoul(value, NULL, 0);
        configx.allowclass.insert((unsigned int)tmp);
    }
    else if (MATCH2(GROUP_BLACKLIST, "GUID"))
    {
        configx.blacklist.insert(value);
    }

    return 1;
}

void read_conf(void)
{
    if (ini_parse(MODULE_NAME".ini", handler, NULL) < 0)
    {
    }
}