# Directory containing wallpapers
WALLPAPER_DIR="/Volumes/Fire/Daily Wallpaper Changer/Wallpapers"
STATE_FILE="$HOME/.wallpaper_state"
SECONDS_PER_DAY=86400


# Get current timestamp
CURRENT_TIME=$(date +%s)


# Load last change time and wallpaper index from state file
if [[ -f "$STATE_FILE" ]]; then
    source "$STATE_FILE"
else
    LAST_CHANGE_TIME=0
    WALLPAPER_INDEX=0
fi


# Function to list all wallpapers in the directory
function list_wallpapers() {
    echo "Available Wallpapers:"
    find "$WALLPAPER_DIR" -type f \( -iname "*.jpg" -o -iname "*.png" -o -iname "*.bmp" \)
}


# Function to change wallpaper on macOS
function change_wallpaper() {
    WALLPAPERS=($(find "$WALLPAPER_DIR" -type f \( -iname "*.jpg" -o -iname "*.png" -o -iname "*.bmp" \)))
    NUM_WALLPAPERS=${#WALLPAPERS[@]}
    
    if [[ $NUM_WALLPAPERS -eq 0 ]]; then
        echo "No wallpapers found in $WALLPAPER_DIR"
        exit 1
    fi


    WALLPAPER=${WALLPAPERS[$WALLPAPER_INDEX]}
    
    osascript -e "tell application \"System Events\" to tell every desktop to set picture to \"$WALLPAPER\""


    LAST_CHANGE_TIME=$CURRENT_TIME
    WALLPAPER_INDEX=$(( (WALLPAPER_INDEX + 1) % NUM_WALLPAPERS ))
    echo "LAST_CHANGE_TIME=$LAST_CHANGE_TIME" > "$STATE_FILE"
    echo "WALLPAPER_INDEX=$WALLPAPER_INDEX" >> "$STATE_FILE"
}


if [[ $((CURRENT_TIME - LAST_CHANGE_TIME)) -ge $SECONDS_PER_DAY ]] || [[ $LAST_CHANGE_TIME -eq 0 ]]; then
    change_wallpaper
else
    echo "Wallpaper will change in $(( (SECONDS_PER_DAY - (CURRENT_TIME - LAST_CHANGE_TIME)) / 3600 )) hours."
fi
