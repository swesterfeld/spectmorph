#!/bin/bash

SMFILEDIALOG=$(echo $0 | sed s/.sh//g)
if [ "x$ARDOUR_BUNDLED" != "x" ]; then
  # at least kxStudio Ardour5.12.0 messes with LD_LIBRARY_PATH, which prevents the file dialog from opening
  # so we try to undo the modification before starting the file dialog
  OLD_ENV=$(echo "$PREBUNDLE_ENV" | grep ^LD_LIBRARY_PATH=)
  if [ "x$OLD_ENV" != "x" ]; then
    exec env "$OLD_ENV" "$SMFILEDIALOG" "$@"
  fi
fi

exec "$SMFILEDIALOG" "$@"
