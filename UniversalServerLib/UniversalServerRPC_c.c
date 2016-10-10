

/* this ALWAYS GENERATED file contains the RPC client stubs */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Sun Oct 09 21:53:13 2016
 */
/* Compiler settings for UniversalServerRPC.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#if !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_ARM_)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#include <string.h>

#include "UniversalServerRPC_h.h"

#define TYPE_FORMAT_STRING_SIZE   47                                
#define PROC_FORMAT_STRING_SIZE   109                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _UniversalServerRPC_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } UniversalServerRPC_MIDL_TYPE_FORMAT_STRING;

typedef struct _UniversalServerRPC_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } UniversalServerRPC_MIDL_PROC_FORMAT_STRING;

typedef struct _UniversalServerRPC_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } UniversalServerRPC_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const UniversalServerRPC_MIDL_TYPE_FORMAT_STRING UniversalServerRPC__MIDL_TypeFormatString;
extern const UniversalServerRPC_MIDL_PROC_FORMAT_STRING UniversalServerRPC__MIDL_ProcFormatString;
extern const UniversalServerRPC_MIDL_EXPR_FORMAT_STRING UniversalServerRPC__MIDL_ExprFormatString;

#define GENERIC_BINDING_TABLE_SIZE   0            


/* Standard interface: UniversalServerRPC, ver. 1.0,
   GUID={0xba209999,0x0c6c,0x11d2,{0x97,0xcf,0x00,0xc0,0x4f,0x8e,0xea,0x68}} */



static const RPC_CLIENT_INTERFACE UniversalServerRPC___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xba209999,0x0c6c,0x11d2,{0x97,0xcf,0x00,0xc0,0x4f,0x8e,0xea,0x68}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE UniversalServerRPC_v1_0_c_ifspec = (RPC_IF_HANDLE)& UniversalServerRPC___RpcClientInterface;

extern const MIDL_STUB_DESC UniversalServerRPC_StubDesc;

static RPC_BINDING_HANDLE UniversalServerRPC__MIDL_AutoBindHandle;


void RunScriptFile( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptFile)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&IDL_handle);
    
}


void RunScriptString( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptString,
    /* [retval][out] */ BSTR *pstrResult)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[34],
                  ( unsigned char * )&IDL_handle);
    
}


void EnqueueWorkerThreadScriptString( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptString)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[74],
                  ( unsigned char * )&IDL_handle);
    
}

extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const UniversalServerRPC_MIDL_PROC_FORMAT_STRING UniversalServerRPC__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure RunScriptFile */

			0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x0 ),	/* 0 */
/* 16 */	NdrFcShort( 0x0 ),	/* 0 */
/* 18 */	0x42,		/* Oi2 Flags:  clt must size, has ext, */
			0x1,		/* 1 */
/* 20 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 28 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

	/* Procedure RunScriptString */


	/* Parameter szScriptFile */

/* 34 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 36 */	NdrFcLong( 0x0 ),	/* 0 */
/* 40 */	NdrFcShort( 0x1 ),	/* 1 */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 46 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 48 */	NdrFcShort( 0x0 ),	/* 0 */
/* 50 */	NdrFcShort( 0x0 ),	/* 0 */
/* 52 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x2,		/* 2 */
/* 54 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 56 */	NdrFcShort( 0x1 ),	/* 1 */
/* 58 */	NdrFcShort( 0x0 ),	/* 0 */
/* 60 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 62 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 64 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 66 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

	/* Parameter szScriptString */

/* 68 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 70 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 72 */	NdrFcShort( 0x24 ),	/* Type Offset=36 */

	/* Procedure EnqueueWorkerThreadScriptString */


	/* Parameter pstrResult */

/* 74 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 76 */	NdrFcLong( 0x0 ),	/* 0 */
/* 80 */	NdrFcShort( 0x2 ),	/* 2 */
/* 82 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 84 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 86 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 88 */	NdrFcShort( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x0 ),	/* 0 */
/* 92 */	0x42,		/* Oi2 Flags:  clt must size, has ext, */
			0x1,		/* 1 */
/* 94 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 96 */	NdrFcShort( 0x0 ),	/* 0 */
/* 98 */	NdrFcShort( 0x0 ),	/* 0 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 102 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 104 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 106 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

			0x0
        }
    };

static const UniversalServerRPC_MIDL_TYPE_FORMAT_STRING UniversalServerRPC__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  4 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  6 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/*  8 */	NdrFcShort( 0x1c ),	/* Offset= 28 (36) */
/* 10 */	
			0x12, 0x0,	/* FC_UP */
/* 12 */	NdrFcShort( 0xe ),	/* Offset= 14 (26) */
/* 14 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 16 */	NdrFcShort( 0x2 ),	/* 2 */
/* 18 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 20 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 22 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 24 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 26 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 28 */	NdrFcShort( 0x8 ),	/* 8 */
/* 30 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (14) */
/* 32 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 34 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 36 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 38 */	NdrFcShort( 0x0 ),	/* 0 */
/* 40 */	NdrFcShort( 0x4 ),	/* 4 */
/* 42 */	NdrFcShort( 0x0 ),	/* 0 */
/* 44 */	NdrFcShort( 0xffde ),	/* Offset= -34 (10) */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };


static const unsigned short UniversalServerRPC_FormatStringOffsetTable[] =
    {
    0,
    34,
    74
    };


static const MIDL_STUB_DESC UniversalServerRPC_StubDesc = 
    {
    (void *)& UniversalServerRPC___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &UniversalServerRPC__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    UniversalServerRPC__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x800025b, /* MIDL Version 8.0.603 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_ARM_) */

