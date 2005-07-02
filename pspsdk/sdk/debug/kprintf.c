#include <pspkernel.h>
#include <pspdebug.h>

#define REGISTER_FUNC_MOD "sceSystemMemoryManager"
#define REGISTER_FUNC_LIB "KDebugForKernel"
#define REGISTER_FUNC_NID 0x7CEB2C09

static PspDebugKprintfHandler curr_handler;

static void _pspDebugDefaultKprintfHandler(const char *format, u32 *args)
{
	pspDebugScreenPrintf(format, args[0], args[1], args[2], args[3]);
}

static void _pspDebugKprintfHandler(void *arg, const char *format, u32 *args)
{
	if(curr_handler != NULL)
	{
		curr_handler(format, args);
	}
	else
	{
		_pspDebugDefaultKprintfHandler(format, args);
	}
}

int pspDebugInstallKprintfHandler(PspDebugKprintfHandler handler)
{
	SceModuleInfo *mod;
	int (*sceKernelRegisterKprintfHandler)(void *func, void *args);

	mod = pspDebugFindModule(REGISTER_FUNC_MOD);
	if(mod != NULL)
	{
		sceKernelRegisterKprintfHandler = pspDebugFindExportedFunction(mod, REGISTER_FUNC_LIB, REGISTER_FUNC_NID);
	}

	if((mod == NULL) || (sceKernelRegisterKprintfHandler == NULL))
	{
		pspDebugScreenPrintf("Error findind Kprintf register function\n");
		return -1;
	}

	return sceKernelRegisterKprintfHandler(_pspDebugKprintfHandler, NULL);
}