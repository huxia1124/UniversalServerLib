

/* this ALWAYS GENERATED file contains the RPC client stubs */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Sat Oct 29 17:09:10 2016
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

#define TYPE_FORMAT_STRING_SIZE   1069                              
#define PROC_FORMAT_STRING_SIZE   241                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

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


void GetSharedDataTreeNodes( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [out] */ SAFEARRAY * *nodeNames,
    /* [out] */ SAFEARRAY * *nodeTypes,
    /* [out] */ SAFEARRAY * *nodeFlags)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[108],
                  ( unsigned char * )&IDL_handle);
    
}


void GetSharedDataTreeNodeStringValue( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [retval][out] */ BSTR *pstrValue)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[160],
                  ( unsigned char * )&IDL_handle);
    
}


void GetSharedDataTreeNodeValues( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [out] */ SAFEARRAY * *nodeValues)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&UniversalServerRPC_StubDesc,
                  (PFORMAT_STRING) &UniversalServerRPC__MIDL_ProcFormatString.Format[200],
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

	/* Procedure GetSharedDataTreeNodes */


	/* Parameter szScriptString */

/* 108 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 110 */	NdrFcLong( 0x0 ),	/* 0 */
/* 114 */	NdrFcShort( 0x3 ),	/* 3 */
/* 116 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 118 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 120 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 122 */	NdrFcShort( 0x0 ),	/* 0 */
/* 124 */	NdrFcShort( 0x0 ),	/* 0 */
/* 126 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x4,		/* 4 */
/* 128 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 130 */	NdrFcShort( 0x1 ),	/* 1 */
/* 132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 136 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 138 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 140 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

	/* Parameter szPath */

/* 142 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 144 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 146 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */

	/* Parameter nodeNames */

/* 148 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 150 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 152 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */

	/* Parameter nodeTypes */

/* 154 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 156 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 158 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */

	/* Procedure GetSharedDataTreeNodeStringValue */


	/* Parameter nodeFlags */

/* 160 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 162 */	NdrFcLong( 0x0 ),	/* 0 */
/* 166 */	NdrFcShort( 0x4 ),	/* 4 */
/* 168 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 170 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 172 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 174 */	NdrFcShort( 0x0 ),	/* 0 */
/* 176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 178 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x2,		/* 2 */
/* 180 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 182 */	NdrFcShort( 0x1 ),	/* 1 */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 188 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 190 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 192 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

	/* Parameter szPath */

/* 194 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 196 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 198 */	NdrFcShort( 0x24 ),	/* Type Offset=36 */

	/* Procedure GetSharedDataTreeNodeValues */


	/* Parameter pstrValue */

/* 200 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 202 */	NdrFcLong( 0x0 ),	/* 0 */
/* 206 */	NdrFcShort( 0x5 ),	/* 5 */
/* 208 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 210 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 212 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 214 */	NdrFcShort( 0x0 ),	/* 0 */
/* 216 */	NdrFcShort( 0x0 ),	/* 0 */
/* 218 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x2,		/* 2 */
/* 220 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 222 */	NdrFcShort( 0x1 ),	/* 1 */
/* 224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 226 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 228 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 230 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 232 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */

	/* Parameter szPath */

/* 234 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 236 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 238 */	NdrFcShort( 0x422 ),	/* Type Offset=1058 */

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
/* 46 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 48 */	NdrFcShort( 0x3f2 ),	/* Offset= 1010 (1058) */
/* 50 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 52 */	NdrFcShort( 0x2 ),	/* Offset= 2 (54) */
/* 54 */	
			0x12, 0x0,	/* FC_UP */
/* 56 */	NdrFcShort( 0x3d8 ),	/* Offset= 984 (1040) */
/* 58 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x49,		/* 73 */
/* 60 */	NdrFcShort( 0x18 ),	/* 24 */
/* 62 */	NdrFcShort( 0xa ),	/* 10 */
/* 64 */	NdrFcLong( 0x8 ),	/* 8 */
/* 68 */	NdrFcShort( 0x5a ),	/* Offset= 90 (158) */
/* 70 */	NdrFcLong( 0xd ),	/* 13 */
/* 74 */	NdrFcShort( 0x90 ),	/* Offset= 144 (218) */
/* 76 */	NdrFcLong( 0x9 ),	/* 9 */
/* 80 */	NdrFcShort( 0xc2 ),	/* Offset= 194 (274) */
/* 82 */	NdrFcLong( 0xc ),	/* 12 */
/* 86 */	NdrFcShort( 0x2bc ),	/* Offset= 700 (786) */
/* 88 */	NdrFcLong( 0x24 ),	/* 36 */
/* 92 */	NdrFcShort( 0x2e6 ),	/* Offset= 742 (834) */
/* 94 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 98 */	NdrFcShort( 0x302 ),	/* Offset= 770 (868) */
/* 100 */	NdrFcLong( 0x10 ),	/* 16 */
/* 104 */	NdrFcShort( 0x31c ),	/* Offset= 796 (900) */
/* 106 */	NdrFcLong( 0x2 ),	/* 2 */
/* 110 */	NdrFcShort( 0x336 ),	/* Offset= 822 (932) */
/* 112 */	NdrFcLong( 0x3 ),	/* 3 */
/* 116 */	NdrFcShort( 0x350 ),	/* Offset= 848 (964) */
/* 118 */	NdrFcLong( 0x14 ),	/* 20 */
/* 122 */	NdrFcShort( 0x36a ),	/* Offset= 874 (996) */
/* 124 */	NdrFcShort( 0xffff ),	/* Offset= -1 (123) */
/* 126 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 128 */	NdrFcShort( 0x4 ),	/* 4 */
/* 130 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 134 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 136 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 138 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 140 */	NdrFcShort( 0x4 ),	/* 4 */
/* 142 */	NdrFcShort( 0x0 ),	/* 0 */
/* 144 */	NdrFcShort( 0x1 ),	/* 1 */
/* 146 */	NdrFcShort( 0x0 ),	/* 0 */
/* 148 */	NdrFcShort( 0x0 ),	/* 0 */
/* 150 */	0x12, 0x0,	/* FC_UP */
/* 152 */	NdrFcShort( 0xff82 ),	/* Offset= -126 (26) */
/* 154 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 156 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 158 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 160 */	NdrFcShort( 0x8 ),	/* 8 */
/* 162 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 164 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 166 */	NdrFcShort( 0x4 ),	/* 4 */
/* 168 */	NdrFcShort( 0x4 ),	/* 4 */
/* 170 */	0x11, 0x0,	/* FC_RP */
/* 172 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (126) */
/* 174 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 176 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 178 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 180 */	NdrFcLong( 0x0 ),	/* 0 */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 188 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 190 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 192 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 194 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 196 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 198 */	NdrFcShort( 0x0 ),	/* 0 */
/* 200 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 202 */	NdrFcShort( 0x0 ),	/* 0 */
/* 204 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 206 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 210 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 212 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 214 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (178) */
/* 216 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 218 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 220 */	NdrFcShort( 0x8 ),	/* 8 */
/* 222 */	NdrFcShort( 0x0 ),	/* 0 */
/* 224 */	NdrFcShort( 0x6 ),	/* Offset= 6 (230) */
/* 226 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 228 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 230 */	
			0x11, 0x0,	/* FC_RP */
/* 232 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (196) */
/* 234 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 236 */	NdrFcLong( 0x20400 ),	/* 132096 */
/* 240 */	NdrFcShort( 0x0 ),	/* 0 */
/* 242 */	NdrFcShort( 0x0 ),	/* 0 */
/* 244 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 246 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 248 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 250 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 252 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 254 */	NdrFcShort( 0x0 ),	/* 0 */
/* 256 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 258 */	NdrFcShort( 0x0 ),	/* 0 */
/* 260 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 262 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 266 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 268 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 270 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (234) */
/* 272 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 274 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 278 */	NdrFcShort( 0x0 ),	/* 0 */
/* 280 */	NdrFcShort( 0x6 ),	/* Offset= 6 (286) */
/* 282 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 284 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 286 */	
			0x11, 0x0,	/* FC_RP */
/* 288 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (252) */
/* 290 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 292 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 294 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 296 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 298 */	NdrFcShort( 0x2 ),	/* Offset= 2 (300) */
/* 300 */	NdrFcShort( 0x10 ),	/* 16 */
/* 302 */	NdrFcShort( 0x2f ),	/* 47 */
/* 304 */	NdrFcLong( 0x14 ),	/* 20 */
/* 308 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 310 */	NdrFcLong( 0x3 ),	/* 3 */
/* 314 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 316 */	NdrFcLong( 0x11 ),	/* 17 */
/* 320 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 322 */	NdrFcLong( 0x2 ),	/* 2 */
/* 326 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 328 */	NdrFcLong( 0x4 ),	/* 4 */
/* 332 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 334 */	NdrFcLong( 0x5 ),	/* 5 */
/* 338 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 340 */	NdrFcLong( 0xb ),	/* 11 */
/* 344 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 346 */	NdrFcLong( 0xa ),	/* 10 */
/* 350 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 352 */	NdrFcLong( 0x6 ),	/* 6 */
/* 356 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (588) */
/* 358 */	NdrFcLong( 0x7 ),	/* 7 */
/* 362 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 364 */	NdrFcLong( 0x8 ),	/* 8 */
/* 368 */	NdrFcShort( 0xfe9a ),	/* Offset= -358 (10) */
/* 370 */	NdrFcLong( 0xd ),	/* 13 */
/* 374 */	NdrFcShort( 0xff3c ),	/* Offset= -196 (178) */
/* 376 */	NdrFcLong( 0x9 ),	/* 9 */
/* 380 */	NdrFcShort( 0xff6e ),	/* Offset= -146 (234) */
/* 382 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 386 */	NdrFcShort( 0xd0 ),	/* Offset= 208 (594) */
/* 388 */	NdrFcLong( 0x24 ),	/* 36 */
/* 392 */	NdrFcShort( 0xd2 ),	/* Offset= 210 (602) */
/* 394 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 398 */	NdrFcShort( 0xcc ),	/* Offset= 204 (602) */
/* 400 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 404 */	NdrFcShort( 0xfc ),	/* Offset= 252 (656) */
/* 406 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 410 */	NdrFcShort( 0xfa ),	/* Offset= 250 (660) */
/* 412 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 416 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (664) */
/* 418 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 422 */	NdrFcShort( 0xf6 ),	/* Offset= 246 (668) */
/* 424 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 428 */	NdrFcShort( 0xf4 ),	/* Offset= 244 (672) */
/* 430 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 434 */	NdrFcShort( 0xf2 ),	/* Offset= 242 (676) */
/* 436 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 440 */	NdrFcShort( 0xdc ),	/* Offset= 220 (660) */
/* 442 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 446 */	NdrFcShort( 0xda ),	/* Offset= 218 (664) */
/* 448 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 452 */	NdrFcShort( 0xe4 ),	/* Offset= 228 (680) */
/* 454 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 458 */	NdrFcShort( 0xda ),	/* Offset= 218 (676) */
/* 460 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 464 */	NdrFcShort( 0xdc ),	/* Offset= 220 (684) */
/* 466 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 470 */	NdrFcShort( 0xda ),	/* Offset= 218 (688) */
/* 472 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 476 */	NdrFcShort( 0xd8 ),	/* Offset= 216 (692) */
/* 478 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 482 */	NdrFcShort( 0xd6 ),	/* Offset= 214 (696) */
/* 484 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 488 */	NdrFcShort( 0xdc ),	/* Offset= 220 (708) */
/* 490 */	NdrFcLong( 0x10 ),	/* 16 */
/* 494 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 496 */	NdrFcLong( 0x12 ),	/* 18 */
/* 500 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 502 */	NdrFcLong( 0x13 ),	/* 19 */
/* 506 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 508 */	NdrFcLong( 0x15 ),	/* 21 */
/* 512 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 514 */	NdrFcLong( 0x16 ),	/* 22 */
/* 518 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 520 */	NdrFcLong( 0x17 ),	/* 23 */
/* 524 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 526 */	NdrFcLong( 0xe ),	/* 14 */
/* 530 */	NdrFcShort( 0xba ),	/* Offset= 186 (716) */
/* 532 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 536 */	NdrFcShort( 0xbe ),	/* Offset= 190 (726) */
/* 538 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 542 */	NdrFcShort( 0xbc ),	/* Offset= 188 (730) */
/* 544 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 548 */	NdrFcShort( 0x70 ),	/* Offset= 112 (660) */
/* 550 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 554 */	NdrFcShort( 0x6e ),	/* Offset= 110 (664) */
/* 556 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 560 */	NdrFcShort( 0x6c ),	/* Offset= 108 (668) */
/* 562 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 566 */	NdrFcShort( 0x62 ),	/* Offset= 98 (664) */
/* 568 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 572 */	NdrFcShort( 0x5c ),	/* Offset= 92 (664) */
/* 574 */	NdrFcLong( 0x0 ),	/* 0 */
/* 578 */	NdrFcShort( 0x0 ),	/* Offset= 0 (578) */
/* 580 */	NdrFcLong( 0x1 ),	/* 1 */
/* 584 */	NdrFcShort( 0x0 ),	/* Offset= 0 (584) */
/* 586 */	NdrFcShort( 0xffff ),	/* Offset= -1 (585) */
/* 588 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 590 */	NdrFcShort( 0x8 ),	/* 8 */
/* 592 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 594 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 596 */	NdrFcShort( 0x2 ),	/* Offset= 2 (598) */
/* 598 */	
			0x12, 0x0,	/* FC_UP */
/* 600 */	NdrFcShort( 0x1b8 ),	/* Offset= 440 (1040) */
/* 602 */	
			0x12, 0x0,	/* FC_UP */
/* 604 */	NdrFcShort( 0x20 ),	/* Offset= 32 (636) */
/* 606 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 608 */	NdrFcLong( 0x2f ),	/* 47 */
/* 612 */	NdrFcShort( 0x0 ),	/* 0 */
/* 614 */	NdrFcShort( 0x0 ),	/* 0 */
/* 616 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 618 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 620 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 622 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 624 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 626 */	NdrFcShort( 0x1 ),	/* 1 */
/* 628 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 630 */	NdrFcShort( 0x4 ),	/* 4 */
/* 632 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 634 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 636 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 638 */	NdrFcShort( 0x10 ),	/* 16 */
/* 640 */	NdrFcShort( 0x0 ),	/* 0 */
/* 642 */	NdrFcShort( 0xa ),	/* Offset= 10 (652) */
/* 644 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 646 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 648 */	NdrFcShort( 0xffd6 ),	/* Offset= -42 (606) */
/* 650 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 652 */	
			0x12, 0x0,	/* FC_UP */
/* 654 */	NdrFcShort( 0xffe2 ),	/* Offset= -30 (624) */
/* 656 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 658 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 660 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 662 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 664 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 666 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 668 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 670 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 672 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 674 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 676 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 678 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 680 */	
			0x12, 0x0,	/* FC_UP */
/* 682 */	NdrFcShort( 0xffa2 ),	/* Offset= -94 (588) */
/* 684 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 686 */	NdrFcShort( 0xfd5c ),	/* Offset= -676 (10) */
/* 688 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 690 */	NdrFcShort( 0xfe00 ),	/* Offset= -512 (178) */
/* 692 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 694 */	NdrFcShort( 0xfe34 ),	/* Offset= -460 (234) */
/* 696 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 698 */	NdrFcShort( 0x2 ),	/* Offset= 2 (700) */
/* 700 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 702 */	NdrFcShort( 0x2 ),	/* Offset= 2 (704) */
/* 704 */	
			0x12, 0x0,	/* FC_UP */
/* 706 */	NdrFcShort( 0x14e ),	/* Offset= 334 (1040) */
/* 708 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 710 */	NdrFcShort( 0x2 ),	/* Offset= 2 (712) */
/* 712 */	
			0x12, 0x0,	/* FC_UP */
/* 714 */	NdrFcShort( 0x14 ),	/* Offset= 20 (734) */
/* 716 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 718 */	NdrFcShort( 0x10 ),	/* 16 */
/* 720 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 722 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 724 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 726 */	
			0x12, 0x0,	/* FC_UP */
/* 728 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (716) */
/* 730 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 732 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 734 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 736 */	NdrFcShort( 0x20 ),	/* 32 */
/* 738 */	NdrFcShort( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x0 ),	/* Offset= 0 (740) */
/* 742 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 744 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 746 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 748 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 750 */	NdrFcShort( 0xfe34 ),	/* Offset= -460 (290) */
/* 752 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 754 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 756 */	NdrFcShort( 0x4 ),	/* 4 */
/* 758 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 760 */	NdrFcShort( 0x0 ),	/* 0 */
/* 762 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 764 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 766 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 768 */	NdrFcShort( 0x4 ),	/* 4 */
/* 770 */	NdrFcShort( 0x0 ),	/* 0 */
/* 772 */	NdrFcShort( 0x1 ),	/* 1 */
/* 774 */	NdrFcShort( 0x0 ),	/* 0 */
/* 776 */	NdrFcShort( 0x0 ),	/* 0 */
/* 778 */	0x12, 0x0,	/* FC_UP */
/* 780 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (734) */
/* 782 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 784 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 786 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 788 */	NdrFcShort( 0x8 ),	/* 8 */
/* 790 */	NdrFcShort( 0x0 ),	/* 0 */
/* 792 */	NdrFcShort( 0x6 ),	/* Offset= 6 (798) */
/* 794 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 796 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 798 */	
			0x11, 0x0,	/* FC_RP */
/* 800 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (754) */
/* 802 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 804 */	NdrFcShort( 0x4 ),	/* 4 */
/* 806 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 808 */	NdrFcShort( 0x0 ),	/* 0 */
/* 810 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 812 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 814 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 816 */	NdrFcShort( 0x4 ),	/* 4 */
/* 818 */	NdrFcShort( 0x0 ),	/* 0 */
/* 820 */	NdrFcShort( 0x1 ),	/* 1 */
/* 822 */	NdrFcShort( 0x0 ),	/* 0 */
/* 824 */	NdrFcShort( 0x0 ),	/* 0 */
/* 826 */	0x12, 0x0,	/* FC_UP */
/* 828 */	NdrFcShort( 0xff40 ),	/* Offset= -192 (636) */
/* 830 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 832 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 834 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 836 */	NdrFcShort( 0x8 ),	/* 8 */
/* 838 */	NdrFcShort( 0x0 ),	/* 0 */
/* 840 */	NdrFcShort( 0x6 ),	/* Offset= 6 (846) */
/* 842 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 844 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 846 */	
			0x11, 0x0,	/* FC_RP */
/* 848 */	NdrFcShort( 0xffd2 ),	/* Offset= -46 (802) */
/* 850 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 852 */	NdrFcShort( 0x8 ),	/* 8 */
/* 854 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 856 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 858 */	NdrFcShort( 0x10 ),	/* 16 */
/* 860 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 862 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 864 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (850) */
			0x5b,		/* FC_END */
/* 868 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 870 */	NdrFcShort( 0x18 ),	/* 24 */
/* 872 */	NdrFcShort( 0x0 ),	/* 0 */
/* 874 */	NdrFcShort( 0xa ),	/* Offset= 10 (884) */
/* 876 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 878 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 880 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (856) */
/* 882 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 884 */	
			0x11, 0x0,	/* FC_RP */
/* 886 */	NdrFcShort( 0xfd4e ),	/* Offset= -690 (196) */
/* 888 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 890 */	NdrFcShort( 0x1 ),	/* 1 */
/* 892 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 894 */	NdrFcShort( 0x0 ),	/* 0 */
/* 896 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 898 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 900 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 902 */	NdrFcShort( 0x8 ),	/* 8 */
/* 904 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 906 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 908 */	NdrFcShort( 0x4 ),	/* 4 */
/* 910 */	NdrFcShort( 0x4 ),	/* 4 */
/* 912 */	0x12, 0x0,	/* FC_UP */
/* 914 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (888) */
/* 916 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 918 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 920 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 922 */	NdrFcShort( 0x2 ),	/* 2 */
/* 924 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 926 */	NdrFcShort( 0x0 ),	/* 0 */
/* 928 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 930 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 932 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 934 */	NdrFcShort( 0x8 ),	/* 8 */
/* 936 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 938 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 940 */	NdrFcShort( 0x4 ),	/* 4 */
/* 942 */	NdrFcShort( 0x4 ),	/* 4 */
/* 944 */	0x12, 0x0,	/* FC_UP */
/* 946 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (920) */
/* 948 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 950 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 952 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 954 */	NdrFcShort( 0x4 ),	/* 4 */
/* 956 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 958 */	NdrFcShort( 0x0 ),	/* 0 */
/* 960 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 962 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 964 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 966 */	NdrFcShort( 0x8 ),	/* 8 */
/* 968 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 970 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 972 */	NdrFcShort( 0x4 ),	/* 4 */
/* 974 */	NdrFcShort( 0x4 ),	/* 4 */
/* 976 */	0x12, 0x0,	/* FC_UP */
/* 978 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (952) */
/* 980 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 982 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 984 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 986 */	NdrFcShort( 0x8 ),	/* 8 */
/* 988 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 990 */	NdrFcShort( 0x0 ),	/* 0 */
/* 992 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 994 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 996 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 998 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1000 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1002 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1004 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1006 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1008 */	0x12, 0x0,	/* FC_UP */
/* 1010 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (984) */
/* 1012 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1014 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1016 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 1018 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1020 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1022 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1024 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1026 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1028 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 1030 */	NdrFcShort( 0xffd8 ),	/* -40 */
/* 1032 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 1034 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1036 */	NdrFcShort( 0xffec ),	/* Offset= -20 (1016) */
/* 1038 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1040 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1042 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1044 */	NdrFcShort( 0xffec ),	/* Offset= -20 (1024) */
/* 1046 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1046) */
/* 1048 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1050 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1052 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1054 */	NdrFcShort( 0xfc1c ),	/* Offset= -996 (58) */
/* 1056 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1058 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1060 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1062 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1064 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1066 */	NdrFcShort( 0xfc08 ),	/* Offset= -1016 (50) */

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
            },
            {
            LPSAFEARRAY_UserSize
            ,LPSAFEARRAY_UserMarshal
            ,LPSAFEARRAY_UserUnmarshal
            ,LPSAFEARRAY_UserFree
            }

        };


static const unsigned short UniversalServerRPC_FormatStringOffsetTable[] =
    {
    0,
    34,
    74,
    108,
    160,
    200
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

