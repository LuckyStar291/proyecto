PART=TM4C1294NCPDT
ROOT=../../../..
include ${ROOT}/makedefs
IPATH=../../../..
all: ${COMPILER}
all: ${COMPILER}/distancia.axf
clean:
	@rm -rf ${COMPILER} ${wildcard *~}
${COMPILER}:
	@mkdir -p ${COMPILER}
${COMPILER}/distancia.axf: ${COMPILER}/distancia.o
${COMPILER}/distancia.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/distancia.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/distancia.axf: ${ROOT}/utils/uartstdio.c  # Añadir uartstdio.c
${COMPILER}/distancia.axf: distancia.ld
SCATTERgcc_distancia=distancia.ld
ENTRY_distancia=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C129_RA1
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
