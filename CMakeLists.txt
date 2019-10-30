
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

project(cntlm VERSION 0.93.5)

add_subdirectory(src)
add_subdirectory(doc)


set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Colin Graf <colin.graf@sovereign-labs.com>")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Authenticating HTTP accelerator for NTLM secured proxies")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/craflin/cntlm")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "adduser, libc6 (>= 2.15)")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/src/debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/src/debian/prerm;${CMAKE_CURRENT_SOURCE_DIR}/src/debian/postrm")

include(CPack)

install(FILES etc/cntlm.conf DESTINATION etc)
install(FILES etc/default/cntlm DESTINATION etc/default)
install(PROGRAMS etc/init.d/cntlm DESTINATION etc/init.d)