cmake_minimum_required(VERSION 3.22.1)

project("NGenXXJNI")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${CMAKE_PROJECT_NAME} SHARED NGenXXJNI.cxx)

set(libDir ${PROJECT_SOURCE_DIR}/../../../../../../build.Android/output/libs/${ANDROID_ABI})

#Find libcrypto
set(libcryptoPath ${libDir}/libcrypto.a)
add_library(libcrypto STATIC IMPORTED)
set_property(TARGET libcrypto PROPERTY IMPORTED_LOCATION ${libcryptoPath})

#Find libssl
set(libsslPath ${libDir}/libssl.a)
add_library(libssl STATIC IMPORTED)
set_property(TARGET libssl PROPERTY IMPORTED_LOCATION ${libsslPath})

#Find libcurl
set(libcurlPath ${libDir}/libcurl.a)
add_library(libcurl STATIC IMPORTED)
set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION ${libcurlPath})

#Find liblua
set(libluaPath ${libDir}/liblua.a)
add_library(liblua STATIC IMPORTED)
set_property(TARGET liblua PROPERTY IMPORTED_LOCATION ${libluaPath})

#Find libquickjs
set(libquickjsPath ${libDir}/libqjs.a)
add_library(libquickjs STATIC IMPORTED)
set_property(TARGET libquickjs PROPERTY IMPORTED_LOCATION ${libquickjsPath})

#Find libspdlog
set(libspdlogPath ${libDir}/libspdlog.a)
add_library(libspdlog STATIC IMPORTED)
set_property(TARGET libspdlog PROPERTY IMPORTED_LOCATION ${libspdlogPath})

#Find libuv
set(libuvPath ${libDir}/libuv.a)
add_library(libuv STATIC IMPORTED)
set_property(TARGET libuv PROPERTY IMPORTED_LOCATION ${libuvPath})

#Find libcjson
set(libcjsonPath ${libDir}/libcjson.a)
add_library(libcjson STATIC IMPORTED)
set_property(TARGET libcjson PROPERTY IMPORTED_LOCATION ${libcjsonPath})

#Find libsqlite
set(libsqlitePath ${libDir}/libsqlite3.a)
add_library(libsqlite STATIC IMPORTED)
set_property(TARGET libsqlite PROPERTY IMPORTED_LOCATION ${libsqlitePath})

#Find AdaURL
set(libadaurlPath ${libDir}/libada.a)
add_library(libadaurl STATIC IMPORTED)
set_property(TARGET libadaurl PROPERTY IMPORTED_LOCATION ${libadaurlPath})

#Find libmmkvcore
set(libmmkvcorePath ${libDir}/libmmkvcore.a)
add_library(libmmkvcore STATIC IMPORTED)
set_property(TARGET libmmkvcore PROPERTY IMPORTED_LOCATION ${libmmkvcorePath})

#Find libmmkv
set(libmmkvPath ${libDir}/libmmkv.a)
add_library(libmmkv STATIC IMPORTED)
set_property(TARGET libmmkv PROPERTY IMPORTED_LOCATION ${libmmkvPath})

#Find libNGenXX
set(libNGenXXPath ${libDir}/libNGenXX.a)
add_library(libNGenXX STATIC IMPORTED)
set_property(TARGET libNGenXX PROPERTY IMPORTED_LOCATION ${libNGenXXPath})

target_link_libraries(${CMAKE_PROJECT_NAME}
        libcrypto
        libssl
        libcurl
        liblua
        libquickjs
        libspdlog
        libuv
        libcjson
        libsqlite
        libadaurl
        libmmkvcore
        libmmkv
        libNGenXX
        z
        android
        log
)