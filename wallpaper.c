#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define WALLPAPER_PATH "C:\\Wallpapers\\"
#define STATE_FILE "C:\\Wallpapers\\.wallpaper_state"
#define PATH_SEPARATOR '\\'
#elif __APPLE__
#define WALLPAPER_PATH "/Users/Shared/Wallpapers/"
#define STATE_FILE "/Users/Shared/Wallpapers/.wallpaper_state"
#define PATH_SEPARATOR '/'
#include <unistd.h>
#else
#define WALLPAPER_PATH "/home/user/Wallpapers/"
#define STATE_FILE "/home/user/Wallpapers/.wallpaper_state"
#define PATH_SEPARATOR '/'
#include <unistd.h>
#endif

#define MAX_WALLPAPERS 1000
#define MAX_PATH_LEN 512
#define VERSION "2.0"
#define SECONDS_PER_DAY 86400

// State structure to track wallpaper rotation
typedef struct {
    char last_wallpaper[MAX_PATH_LEN];
    time_t last_change_time;
    int wallpaper_index;
    int total_wallpapers;
} WallpaperState;

// Supported image extensions
const char *SUPPORTED_EXTENSIONS[] = {
    ".jpg", ".jpeg", ".png", ".bmp", ".gif", ".webp", ".tiff", ".tif", NULL
};

// Print program header
void printBanner() {
    printf("\n");
    printf("====================================\n");
    printf("  Daily Wallpaper Changer v%s\n", VERSION);
    printf("  Cross-Platform Wallpaper Manager\n");
    printf("====================================\n\n");
}

// Check if filename has supported image extension
int isImageFile(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    for (int i = 0; SUPPORTED_EXTENSIONS[i] != NULL; i++) {
        if (strcasecmp(ext, SUPPORTED_EXTENSIONS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Check if file or directory exists
int fileExists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Create directory if it doesn't exist
void createDirectory(const char *path) {
#ifdef _WIN32
    CreateDirectoryA(path, NULL);
#else
    mkdir(path, 0755);
#endif
}

// Load previous state from file
void loadState(WallpaperState *state) {
    FILE *file = fopen(STATE_FILE, "r");
    if (!file) {
        // Initialize default state
        memset(state, 0, sizeof(WallpaperState));
        state->wallpaper_index = 0;
        state->total_wallpapers = 0;
        state->last_change_time = 0;
        return;
    }
    
    fscanf(file, "%ld\n", &state->last_change_time);
    fscanf(file, "%d\n", &state->wallpaper_index);
    fscanf(file, "%d\n", &state->total_wallpapers);
    fgets(state->last_wallpaper, MAX_PATH_LEN, file);
    
    // Remove trailing newline
    state->last_wallpaper[strcspn(state->last_wallpaper, "\n")] = 0;
    
    fclose(file);
    printf("[INFO] State loaded: Index %d/%d\n", state->wallpaper_index, state->total_wallpapers);
}

// Save current state to file
void saveState(const WallpaperState *state) {
    FILE *file = fopen(STATE_FILE, "w");
    if (!file) {
        perror("[WARNING] Could not save state");
        return;
    }
    
    fprintf(file, "%ld\n", state->last_change_time);
    fprintf(file, "%d\n", state->wallpaper_index);
    fprintf(file, "%d\n", state->total_wallpapers);
    fprintf(file, "%s\n", state->last_wallpaper);
    
    fclose(file);
}

// Collect all wallpapers from directory
int collectWallpapers(char wallpapers[][MAX_PATH_LEN]) {
    int count = 0;
    DIR *dir;
    struct dirent *ent;

    printf("[INFO] Scanning: %s\n", WALLPAPER_PATH);
    
    dir = opendir(WALLPAPER_PATH);
    if (!dir) {
        fprintf(stderr, "[ERROR] Cannot open wallpaper directory: %s\n", WALLPAPER_PATH);
        fprintf(stderr, "[ERROR] Create the directory first!\n");
        return 0;
    }

    while ((ent = readdir(dir)) != NULL && count < MAX_WALLPAPERS) {
        // Skip hidden files
        if (ent->d_name[0] == '.') continue;
        
        if (isImageFile(ent->d_name)) {
            snprintf(wallpapers[count], MAX_PATH_LEN, "%s%s", WALLPAPER_PATH, ent->d_name);
            
            // Verify file exists
            if (fileExists(wallpapers[count])) {
                count++;
            }
        }
    }
    closedir(dir);

    printf("[INFO] Found %d wallpapers\n", count);
    return count;
}

// Set wallpaper using platform-specific methods
void setWallpaper(const char *imagePath) {
#ifdef _WIN32
    printf("[INFO] Setting wallpaper (Windows)...\n");
    if (SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)imagePath,
                              SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        printf("[SUCCESS] Wallpaper set successfully!\n");
    } else {
        fprintf(stderr, "[ERROR] Failed to set wallpaper\n");
    }
    
#elif __APPLE__
    printf("[INFO] Setting wallpaper (macOS)...\n");
    char command[MAX_PATH_LEN + 256];
    
    // Escape spaces in path
    char escaped_path[MAX_PATH_LEN * 2];
    int j = 0;
    for (int i = 0; imagePath[i] != '\0' && j < MAX_PATH_LEN * 2 - 1; i++) {
        if (imagePath[i] == ' ') {
            escaped_path[j++] = '\\';
        }
        escaped_path[j++] = imagePath[i];
    }
    escaped_path[j] = '\0';
    
    snprintf(command, sizeof(command),
             "osascript -e 'tell application \"System Events\" to tell every desktop to set picture to \"%s\"' > /dev/null 2>&1",
             escaped_path);
    
    if (system(command) == 0) {
        printf("[SUCCESS] Wallpaper set successfully!\n");
    } else {
        fprintf(stderr, "[ERROR] Failed to set wallpaper\n");
    }
    
#else // Linux
    printf("[INFO] Setting wallpaper (Linux)...\n");
    char command[MAX_PATH_LEN + 256];
    
    // Try multiple desktop environments
    snprintf(command, sizeof(command),
             "if command -v gsettings > /dev/null 2>&1; then "
             "  gsettings set org.gnome.desktop.background picture-uri \"file://%s\" && "
             "  gsettings set org.gnome.desktop.background picture-uri-dark \"file://%s\"; "
             "elif command -v feh > /dev/null 2>&1; then "
             "  feh --bg-fill \"%s\"; "
             "elif command -v nitrogen > /dev/null 2>&1; then "
             "  nitrogen --set-zoom-fill \"%s\"; "
             "elif command -v xfconf-query > /dev/null 2>&1; then "
             "  xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitor0/workspace0/last-image -s \"%s\"; "
             "else "
             "  echo '[ERROR] No compatible wallpaper tool found'; "
             "  echo '[ERROR] Install: gsettings, feh, nitrogen, or xfconf-query'; "
             "  exit 1; "
             "fi",
             imagePath, imagePath, imagePath, imagePath, imagePath);
    
    if (system(command) == 0) {
        printf("[SUCCESS] Wallpaper set successfully!\n");
    } else {
        fprintf(stderr, "[ERROR] Failed to set wallpaper\n");
    }
#endif

    printf("[INFO] Current: %s\n", imagePath);
}

// Check if 24 hours have passed since last change
int shouldChangeWallpaper(const WallpaperState *state) {
    time_t now = time(NULL);
    time_t elapsed = now - state->last_change_time;
    
    // Change if more than 24 hours have passed
    return elapsed >= SECONDS_PER_DAY || state->last_change_time == 0;
}

// Rotate to next wallpaper in sequence
void rotateWallpaper(char wallpapers[][MAX_PATH_LEN], int count, WallpaperState *state, int force) {
    if (count == 0) {
        fprintf(stderr, "[ERROR] No wallpapers available\n");
        return;
    }
    
    if (!force && !shouldChangeWallpaper(state)) {
        time_t now = time(NULL);
        int hours_remaining = (SECONDS_PER_DAY - (now - state->last_change_time)) / 3600;
        printf("[INFO] Next change in ~%d hours\n", hours_remaining);
        printf("[INFO] Current: %s\n", state->last_wallpaper);
        return;
    }
    
    // Update state
    state->total_wallpapers = count;
    state->wallpaper_index = (state->wallpaper_index + 1) % count;
    state->last_change_time = time(NULL);
    strncpy(state->last_wallpaper, wallpapers[state->wallpaper_index], MAX_PATH_LEN);
    
    // Set wallpaper
    setWallpaper(state->last_wallpaper);
    
    // Save state
    saveState(state);
    
    printf("[INFO] Progress: %d/%d wallpapers\n", state->wallpaper_index + 1, count);
}

// Set a random wallpaper
void setRandomWallpaper(char wallpapers[][MAX_PATH_LEN], int count, WallpaperState *state) {
    if (count == 0) return;
    
    srand((unsigned int)time(NULL));
    int randomIndex = rand() % count;
    
    state->wallpaper_index = randomIndex;
    state->last_change_time = time(NULL);
    state->total_wallpapers = count;
    strncpy(state->last_wallpaper, wallpapers[randomIndex], MAX_PATH_LEN);
    
    setWallpaper(state->last_wallpaper);
    saveState(state);
    
    printf("[INFO] Random: %d/%d\n", randomIndex + 1, count);
}

// Print usage instructions
void printUsage(const char *program) {
    printf("Usage: %s [options]\n\n", program);
    printf("Options:\n");
    printf("  (no args)    Change wallpaper if 24h elapsed (default)\n");
    printf("  --force      Force change wallpaper now\n");
    printf("  --random     Set random wallpaper\n");
    printf("  --next       Skip to next wallpaper\n");
    printf("  --list       List all available wallpapers\n");
    printf("  --status     Show current status\n");
    printf("  --help       Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s                 # Daily auto-rotation\n", program);
    printf("  %s --force         # Change now\n", program);
    printf("  %s --random        # Pick random wallpaper\n\n", program);
    printf("Setup:\n");
    printf("  1. Create folder: %s\n", WALLPAPER_PATH);
    printf("  2. Add wallpapers (jpg, png, bmp, etc.)\n");
    printf("  3. Run this program daily (cron/Task Scheduler)\n\n");
}

int main(int argc, char *argv[]) {
    printBanner();
    
    // Parse command-line arguments
    int force = 0;
    int random_mode = 0;
    int next_mode = 0;
    int list_mode = 0;
    int status_mode = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--force") == 0) {
            force = 1;
        } else if (strcmp(argv[i], "--random") == 0) {
            random_mode = 1;
        } else if (strcmp(argv[i], "--next") == 0) {
            next_mode = 1;
        } else if (strcmp(argv[i], "--list") == 0) {
            list_mode = 1;
        } else if (strcmp(argv[i], "--status") == 0) {
            status_mode = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "[ERROR] Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Check if wallpaper directory exists
    if (!fileExists(WALLPAPER_PATH)) {
        fprintf(stderr, "[ERROR] Wallpaper directory not found: %s\n", WALLPAPER_PATH);
        fprintf(stderr, "[INFO] Creating directory...\n");
        createDirectory(WALLPAPER_PATH);
        fprintf(stderr, "[SUCCESS] Directory created. Add wallpapers and run again.\n");
        return 1;
    }
    
    // Collect all wallpapers from directory
    char wallpapers[MAX_WALLPAPERS][MAX_PATH_LEN];
    int count = collectWallpapers(wallpapers);
    
    if (count == 0) {
        fprintf(stderr, "[ERROR] No wallpapers found in %s\n", WALLPAPER_PATH);
        fprintf(stderr, "[INFO] Add some images (.jpg, .png, .bmp) and try again.\n");
        return 1;
    }
    
    // Load previous state
    WallpaperState state;
    loadState(&state);
    
    // Handle list mode
    if (list_mode) {
        printf("\n[LIST] Available Wallpapers:\n");
        printf("----------------------------------------\n");
        for (int i = 0; i < count; i++) {
            const char *filename = strrchr(wallpapers[i], PATH_SEPARATOR);
            filename = filename ? filename + 1 : wallpapers[i];
            printf("%3d. %s%s\n", i + 1, filename, 
                   (i == state.wallpaper_index) ? " [CURRENT]" : "");
        }
        printf("----------------------------------------\n");
        return 0;
    }
    
    // Handle status mode
    if (status_mode) {
        printf("\n[STATUS] Current Status:\n");
        printf("----------------------------------------\n");
        printf("Total wallpapers: %d\n", count);
        printf("Current index: %d/%d\n", state.wallpaper_index + 1, count);
        
        if (state.last_change_time > 0) {
            time_t now = time(NULL);
            int hours_ago = (now - state.last_change_time) / 3600;
            int hours_until = (SECONDS_PER_DAY - (now - state.last_change_time)) / 3600;
            
            printf("Last changed: %d hours ago\n", hours_ago);
            printf("Next change: in ~%d hours\n", hours_until > 0 ? hours_until : 0);
            
            const char *filename = strrchr(state.last_wallpaper, PATH_SEPARATOR);
            filename = filename ? filename + 1 : state.last_wallpaper;
            printf("Current: %s\n", filename);
        } else {
            printf("Status: Never changed\n");
        }
        printf("----------------------------------------\n");
        return 0;
    }
    
    // Handle different wallpaper modes
    if (random_mode) {
        printf("\n[MODE] Random Mode\n");
        setRandomWallpaper(wallpapers, count, &state);
    } else if (next_mode) {
        printf("\n[MODE] Next Wallpaper\n");
        rotateWallpaper(wallpapers, count, &state, 1);
    } else {
        printf("\n[MODE] Daily Rotation\n");
        rotateWallpaper(wallpapers, count, &state, force);
    }
    
    printf("\n[SUCCESS] Done!\n");
    return 0;
}
