

set(HEADERS
    list.h
    token.h
    parser.h
    mpool.h
)

set(SRCS
    parser.c
    token.c
    calculator.c
    mpool.c
    ${HEADERS}
)

add_executable(ccalculator ${SRCS})