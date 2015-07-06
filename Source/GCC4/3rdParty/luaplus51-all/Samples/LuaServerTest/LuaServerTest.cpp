#include "LuaRemoteDebuggingServer/LuaRemoteDebuggingServer.h"
#include "LuaPlus/LuaPlus.h"
#include <string.h>

using namespace LuaPlus;

LuaStateOwner g_luaScript;

void LoadScript( LuaState* state, const char* fileName )
{
	FILE* file = fopen( fileName, "rb" );
	if ( !file )
		return;

	long currentPos = ftell( file );
	fseek( file, 0, SEEK_END );
	long size = ftell( file );
	fseek( file, currentPos, SEEK_SET );

	unsigned char* buf = new unsigned char[ size + 1 ];
	fread( buf, size, 1, file );
	fclose( file );

	buf[size] = 0;

	LuaRemoteDebuggingServer::AttachScriptFile( fileName, buf, size );

	state->DoBuffer( (const char*) buf, strlen( (const char *)buf ), fileName );

	delete [] buf;
}


void Run()
{
	LuaRemoteDebuggingServer::RegisterState("State", *g_luaScript);
	LuaRemoteDebuggingServer::Initialize();
	LuaRemoteDebuggingServer::WaitForDebuggerConnection();

	LoadScript( g_luaScript, "test.lua" );

	LuaRemoteDebuggingServer::Shutdown();
}


int main(int /*argc*/, char* /*argv*/[])
{
	Run();

	return 0;
}
