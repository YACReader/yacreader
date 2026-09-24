#! /bin/bash

# exit whenever a command called in this script fails
set -e

appdir=""

show_usage() {
    echo "Usage: bash $0 --appdir <AppDir>"
}

while [ "$1" != "" ]; do
    case "$1" in
        --plugin-api-version)
            echo "0"
            exit 0
            ;;
        --appdir)
            appdir="$2"
            shift
            shift
            ;;
        *)
            echo "Invalid argument: $1"
            echo
            show_usage
            exit 2
    esac
done

if [[ "$appdir" == "" ]]; then
    show_usage
    exit 2
fi

echo "linuxdeploy-plugin-path"
echo "\$LINUXDEPLOY: \"$LINUXDEPLOY\""

# install a path AppRun hook
# usually, they're not supposed to print anything, but remain as silent as possible
set -x
mkdir -p "$appdir"/apprun-hooks
cat > "$appdir"/apprun-hooks/linuxdeploy-plugin-path.sh <<\EOF
export PATH="$APPDIR/usr/bin:$PATH"
EOF
