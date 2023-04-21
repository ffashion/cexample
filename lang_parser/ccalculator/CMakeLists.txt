

set(HEADERS
    list.h
    token.h
    parser.h
)

set(SRCS
    parser.c
    token.c
    calculator.c
    ${HEADERS}
)

add_executable(ccalculator ${SRCS})