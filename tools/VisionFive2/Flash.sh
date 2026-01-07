DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

$DIR/vf2-imager -i "$1" -o build/vf2.img
$DIR/vf2 build/vf2.img
minicom -D /dev/ttyUSB0
