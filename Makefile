include config.mk

${EXECNAME}: ${SRC}
	${CC} ${SRC} -o ${EXECNAME} ${CFLAGS} ${LIBS}

install: ${EXECNAME}
	cp ${EXECNAME} ${INSTALLDIR}

clean:
	rm -rf ${EXECNAME}

uninstall: ${EXECNAME}
	rm ${INSTALLDIR}/${EXECNAME} 
