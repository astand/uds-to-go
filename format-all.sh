#!/bin/bash
RETURN=0
FORMATTER=$1
OPTIONS=$2

if [ ! -f "$FORMATTER" ]; then
  echo "There is no path to the formatter app" >&2
	exit 1
fi

FILES=`find . | grep -P "^(.*\/src).*\.(c|cpp|h)$"`

for FILE in $FILES; do
  $FORMATTER $OPTIONS $FILE
done

exit $RETURN
