#pragma once
#ifndef TEST_FUCNTION_DLL_EXPORTS
#define TEST_FUCNTION_DLL_EXPORTS
#define TEST_FUCNTION_DLL_API __declspec(dllexport)
#else
#define TEST_FUCNTION_DLL_API __declspec(dllimport)
#endif

extern "C" TEST_FUCNTION_DLL_API void testOut();

