#!/usr/bin/env bash

DOMAIN=dbfix
LANG=${2:-"ru"}
POT_FILE=po/${DOMAIN}.pot
PO_FILE=po/${LANG}/${DOMAIN}.po

case "$1" in
        pot)
            mkdir -p po
            echo "Extracting lines from C source files..."
            xgettext --keyword=_ --keyword=N_ --keyword=C_:1c,2 \
             --sort-by-file -C --from-code=UTF-8 \
            -o ${POT_FILE} *.c *.h
            ;;

        po)
            if [ -e ${PO_FILE} ]; then
                echo "Updating ${PO_FILE}..."
                msgmerge --update ${PO_FILE} ${POT_FILE}
            else
                mkdir -p po/${LANG}
                echo "Creating ${PO_FILE}..."
                msginit --input=${POT_FILE} --locale=${LANG} --output=${PO_FILE}
            fi
            ;;

        mo)
            TARGET_DIR=${3:-"misc/locale"}
            OUT_DIR=${TARGET_DIR}/${LANG}/LC_MESSAGES
            MO_FILE=${OUT_DIR}/${DOMAIN}.mo

            echo "Writing ${MO_FILE}..."
            mkdir -p ${OUT_DIR}
            msgfmt --output-file=${MO_FILE} ${PO_FILE}
            ;;

        *)
            echo $"Usage: $0 {pot|po|mo} [LANG] [TARGET_DIR]"
            exit 1
esac