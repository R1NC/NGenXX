cmake_minimum_required(VERSION 3.5.0)

project(ngenxx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(${CMAKE_PROJECT_NAME} SHARED
    napi_util.cxx
    napi_init.cxx
)

set(libDir ${PROJECT_SOURCE_DIR}/../../../../../../build.HarmonyOS/output/libs/arm64-v8a)

#Find crypto
set(cryptoPath ${libDir}/libcrypto.a)
add_library(libcrypto STATIC IMPORTED)
set_property(TARGET libcrypto PROPERTY IMPORTED_LOCATION ${cryptoPath})

#Find ssl
set(sslPath ${libDir}/libssl.a)
add_library(libssl STATIC IMPORTED)
set_property(TARGET libssl PROPERTY IMPORTED_LOCATION ${sslPath})

#Find curl
set(curlPath ${libDir}/libcurl.a)
add_library(libcurl STATIC IMPORTED)
set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${curlPath})

#Find lua
set(luaPath ${libDir}/liblua.a)
add_library(liblua STATIC IMPORTED)
set_property(TARGET liblua PROPERTY IMPORTED_LOCATION ${luaPath})

#Find quickjs
set(quickjsPath ${libDir}/libqjs.a)
add_library(libquickjs STATIC IMPORTED)
set_property(TARGET libquickjs PROPERTY IMPORTED_LOCATION ${quickjsPath})

#Find spdlog
set(spdlogPath ${libDir}/libspdlog.a)
add_library(libspdlog STATIC IMPORTED)
set_property(TARGET libspdlog PROPERTY IMPORTED_LOCATION ${spdlogPath})

#Find cjson
set(cjsonPath ${libDir}/libcjson.a)
add_library(libcjson STATIC IMPORTED)
set_property(TARGET libcjson PROPERTY IMPORTED_LOCATION ${cjsonPath})

#Find sqlite
set(sqlitePath ${libDir}/libsqlite3.a)
add_library(libsqlite STATIC IMPORTED)
set_property(TARGET libsqlite PROPERTY IMPORTED_LOCATION ${sqlitePath})

#Find AdaURL
# set(adaurlPath ${libDir}/libada.a)
# add_library(libadaurl STATIC IMPORTED)
# set_property(TARGET libadaurl PROPERTY IMPORTED_LOCATION ${adaurlPath})

#Find libmmkvcore
set(libmmkvcorePath ${libDir}/libmmkvcore.a)
add_library(libmmkvcore STATIC IMPORTED)
set_property(TARGET libmmkvcore PROPERTY IMPORTED_LOCATION ${libmmkvcorePath})

#Find libmmkv
set(libmmkvPath ${libDir}/libmmkv.a)
add_library(libmmkv STATIC IMPORTED)
set_property(TARGET libmmkv PROPERTY IMPORTED_LOCATION ${libmmkvPath})

#Find NGenXX
set(NGenXXPath ${libDir}/libNGenXX.a)
add_library(libNGenXX STATIC IMPORTED)
set_property(TARGET libNGenXX PROPERTY IMPORTED_LOCATION ${NGenXXPath})

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    libcrypto
    libssl
    libcurl
    liblua
    libquickjs
    libcjson
    libsqlite
    libspdlog
    #libadaurl
    libmmkvcore
    libmmkv
    libNGenXX
    z
    uv
    libace_napi.z.so
    libhilog_ndk.z.so
    libdeviceinfo_ndk.z.so
)