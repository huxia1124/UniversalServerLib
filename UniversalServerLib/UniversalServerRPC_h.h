

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 21:14:07 2038
 */
/* Compiler settings for UniversalServerRPC.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __UniversalServerRPC_h_h__
#define __UniversalServerRPC_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __UniversalServerRPC_INTERFACE_DEFINED__
#define __UniversalServerRPC_INTERFACE_DEFINED__

/* interface UniversalServerRPC */
/* [version][uuid] */ 

void RunScriptFile( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptFile);

void RunScriptString( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptString,
    /* [retval][out] */ BSTR *pstrResult);

void EnqueueWorkerThreadScriptString( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szScriptString);

void GetSharedDataTreeNodes( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [out] */ SAFEARRAY * *nodeNames,
    /* [out] */ SAFEARRAY * *nodeTypes,
    /* [out] */ SAFEARRAY * *nodeFlags);

void GetSharedDataTreeNodeStringValue( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [retval][out] */ BSTR *pstrValue);

void GetSharedDataTreeNodeValues( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ const WCHAR *szPath,
    /* [out] */ SAFEARRAY * *nodeValues);



extern RPC_IF_HANDLE UniversalServerRPC_v1_0_c_ifspec;
extern RPC_IF_HANDLE UniversalServerRPC_v1_0_s_ifspec;
#endif /* __UniversalServerRPC_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize64(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal64(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal64(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree64(     unsigned long *, LPSAFEARRAY * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


