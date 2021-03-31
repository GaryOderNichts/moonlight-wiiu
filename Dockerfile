# build wut
FROM devkitpro/devkitppc:20200730 AS wutbuild

ENV PATH=$DEVKITPPC/bin:$PATH

WORKDIR /
RUN git clone -b wutsocket --single-branch https://github.com/GaryOderNichts/wut
WORKDIR /wut
RUN make -j
RUN make install
WORKDIR /

# set up builder image
FROM devkitpro/devkitppc:20200730 AS builder

RUN apt-get update && apt-get -y install --no-install-recommends wget tar autoconf automake libtool && rm -rf /var/lib/apt/lists/*
COPY --from=wutbuild /opt/devkitpro/wut /opt/devkitpro/wut

# build SDL2
FROM builder AS sdlbuild
ENV WUT_ROOT=$DEVKITPRO/wut

RUN git clone -b wiiu-2.0.9 --single-branch https://github.com/yawut/SDL
WORKDIR /SDL
RUN mkdir build
WORKDIR /SDL/build
RUN cmake .. -DCMAKE_TOOLCHAIN_FILE=$WUT_ROOT/share/wut.toolchain.cmake -DCMAKE_INSTALL_PREFIX=$DEVKITPRO/portlibs/wiiu
RUN make -j && make install
WORKDIR /

# build openssl
FROM builder AS opensslbuild
ARG openssl_ver=1.1.1i

RUN wget https://www.openssl.org/source/openssl-$openssl_ver.tar.gz && mkdir /openssl && tar xf openssl-$openssl_ver.tar.gz -C /openssl --strip-components=1
WORKDIR /openssl

RUN echo 'diff --git a/Configurations/10-main.conf b/Configurations/10-main.conf\n\
index cea4feb9..22339970 100644\n\
--- a/Configurations/10-main.conf\n\
+++ b/Configurations/10-main.conf\n\
@@ -627,6 +627,27 @@ my %targets = (\n\
         shared_extension => ".so",\n\
     },\n\
 \n\
+### Wii U target\n\
+    "wiiu" => {\n\
+        inherit_from     => [ "BASE_unix" ],\n\
+        CC               => "'$DEVKITPPC'/bin/powerpc-eabi-gcc",\n\
+        CXX              => "'$DEVKITPPC'/bin/powerpc-eabi-g++",\n\
+        AR               => "'$DEVKITPPC'/bin/powerpc-eabi-ar",\n\
+        CFLAGS           => picker(default => "-Wall",\n\
+                                   debug   => "-O0 -g",\n\
+                                   release => "-O3"),\n\
+        CXXFLAGS         => picker(default => "-Wall",\n\
+                                   debug   => "-O0 -g",\n\
+                                   release => "-O3"),\n\
+        LDFLAGS          => "-L'$DEVKITPRO'/wut/lib",\n\
+        cflags           => add("-ffunction-sections -D__WIIU__ -D__WUT__"),\n\
+        cxxflags         => add("-std=c++11"),\n\
+        lib_cppflags     => "-DOPENSSL_USE_NODELETE -DB_ENDIAN -I'$DEVKITPRO'/wut/include -DNO_SYS_UN_H -DNO_SYSLOG",\n\
+        ex_libs          => add("-lwut"),\n\
+        bn_ops           => "BN_LLONG RC4_CHAR",\n\
+        asm_arch         => '"'"'ppc32'"'"',\n\
+    },\n\
+\n ####\n #### Variety of LINUX:-)\n ####\n\
diff --git a/crypto/uid.c b/crypto/uid.c\n\
index 65b11710..c3755310 100644\n\
--- a/crypto/uid.c\n\
+++ b/crypto/uid.c\n\
@@ -10,7 +10,7 @@\n #include <openssl/crypto.h>\n #include <openssl/opensslconf.h>\n\
 \n\
-#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_UEFI)\n\
+#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_UEFI) || defined(__WIIU__)\n\
 \n\
 int OPENSSL_issetugid(void)\n\
 {\
' >> wiiu.patch && git apply wiiu.patch

RUN ./Configure wiiu \
no-threads no-shared no-asm no-ui-console no-unit-test no-tests no-buildtest-c++ no-external-tests no-autoload-config \
--with-rand-seed=none -static \
--prefix=/opt/devkitpro/portlibs/wiiu --openssldir=openssldir

RUN make build_generated
RUN make libssl.a libcrypto.a -j
WORKDIR /

# build curl
FROM builder as curlbuild
ARG curl_ver=7.75.0

# copy in openssl
COPY --from=opensslbuild /openssl/libcrypto.a /openssl/libssl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opensslbuild /openssl/include/openssl /opt/devkitpro/portlibs/wiiu/include/openssl/

RUN wget https://curl.se/download/curl-$curl_ver.tar.gz && mkdir /curl && tar xf curl-$curl_ver.tar.gz -C /curl --strip-components=1
WORKDIR /curl

RUN autoreconf -fi
RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--disable-threaded-resolver \
--disable-pthreads \
--with-ssl=$DEVKITPRO/portlibs/wiiu/ \
--disable-ipv6 \
--disable-unix-sockets \
--disable-socketpair \
--disable-ntlm-wb \
CPPFLAGS="-ffunction-sections -D__WIIU__ -I/opt/devkitpro/wut/include" \
CFLAGS="" \
LDFLAGS="-L/opt/devkitpro/wut/lib" \
LIBS="-lwut -lm" \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config

WORKDIR /curl/lib
RUN make -j install
WORKDIR /curl/include
RUN make -j install
WORKDIR /

FROM builder as expatbuild

RUN wget https://github.com/libexpat/libexpat/releases/download/R_2_3_0/expat-2.3.0.tar.gz && mkdir /expat && tar xf expat-2.3.0.tar.gz -C /expat --strip-components=1
WORKDIR /expat

RUN autoreconf -fi
RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--without-examples \
--without-tests \
--without-docbook \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config

RUN make -j && make install
WORKDIR /

FROM builder as opusbuild

RUN wget https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz && mkdir /opus && tar xf opus-1.1.2.tar.gz -C /opus --strip-components=1
WORKDIR /opus

RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--disable-doc \
--disable-extra-programs \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config \
CFLAGS="$CFLAGS -Wno-expansion-to-defined"
RUN make -j && make install

# build final container
FROM devkitpro/devkitppc:20200730 AS final

# copy in wut
COPY --from=wutbuild /opt/devkitpro/wut /opt/devkitpro/wut

# copy in SDL2
COPY --from=sdlbuild /opt/devkitpro/portlibs/wiiu/lib/libSDL2.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=sdlbuild /opt/devkitpro/portlibs/wiiu/include/SDL2 /opt/devkitpro/portlibs/wiiu/include/SDL2/

# copy in openssl
COPY --from=opensslbuild /openssl/libcrypto.a /openssl/libssl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opensslbuild /openssl/include/openssl /opt/devkitpro/portlibs/wiiu/include/openssl/

# copy in curl
COPY --from=curlbuild /opt/devkitpro/portlibs/wiiu/lib/libcurl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=curlbuild /opt/devkitpro/portlibs/wiiu/include/curl /opt/devkitpro/portlibs/wiiu/include/curl/

# copy in expat
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/lib/libexpat.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat.h /opt/devkitpro/portlibs/wiiu/include/expat.h
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat_config.h /opt/devkitpro/portlibs/wiiu/include/expat_config.h
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat_external.h /opt/devkitpro/portlibs/wiiu/include/expat_external.h

# copy in opus
COPY --from=opusbuild /opt/devkitpro/portlibs/wiiu/lib/libopus.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opusbuild /opt/devkitpro/portlibs/wiiu/include/opus /opt/devkitpro/portlibs/wiiu/include/opus/

WORKDIR /project
