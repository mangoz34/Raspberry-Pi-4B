#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>


typedef struct {
    int duration;
    size_t buffer_size;
} thread_args_t;

/**
 * @brief Reads a single line from a system file (sysfs) and logs it. [cite: 162]
 * @param path The absolute path to the system file.
 * @param label The descriptive label for the log output.
 * @param log_fp Pointer to the output report file.
 */
void log_sys_value(const char *path, const char *label, FILE *log_fp) {
    FILE *fp = fopen(path, "r");
    if (fp) {
        char val[128];
        if (fgets(val, sizeof(val), fp)) {
            val[strcspn(val, "\n")] = 0;
            fprintf(log_fp, "%-20s: %s\n", label, val);
            printf("%-20s: %s\n", label, val);
        }
        fclose(fp);
    }
}

/**
 * @brief Thread function that generates mixed CPU/Memory load.
 */
void* stress_worker(void* args) {
    thread_args_t* t_args = (thread_args_t*)args;
    size_t size = t_args->buffer_size;
    uint32_t *src = malloc(size);
    uint32_t *dst = malloc(size);

    if (!src || !dst) return NULL;

    time_t start = time(NULL);
    volatile float dummy = 1.414f;

    while (time(NULL) - start < t_args->duration) {
        for (int i = 0; i < (int)(size / 4); i++) {
            dst[i] = src[i];
            // 更重的運算：加入除法與複合運算
            dummy = (dummy / 1.000001f) + 0.00001f;
        }
    }

    free(src); free(dst);
    return NULL;
}

/**
 * @brief Reads the desired benchmark duration from a configuration file.
 * @details Looks for "benchmark_time=" in config.txt. Defaults to 60s if not found.
 * @return int The duration in seconds.
 */
int read_config_int(const char *key, int default_val) {
    int val = default_val;
    FILE *cf = fopen("config.txt", "r");
    if (cf) {
        char line[128];
        size_t key_len = strlen(key);
        while (fgets(line, sizeof(line), cf)) {
            // 檢查 key= 格式
            if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
                val = atoi(line + key_len + 1);
                break;
            }
        }
        fclose(cf);
    }
    return val;
}

/**
 * @brief Fetches specific hardware data using the vcgencmd utility.
 * @param cmd_type The specific vcgencmd command (e.g., "measure_temp").
 * @param output Buffer to store the resulting value.
 * @param size Size of the output buffer.
 */
void get_vcgen_data(const char *cmd_type, char *output, size_t size) {
    char cmd[128];
    sprintf(cmd, "vcgencmd %s", cmd_type);
    FILE *p = popen(cmd, "r");
    if (p) {
        if (fgets(output, size, p)) {
            char *eq = strchr(output, '=');
            if (eq) memmove(output, eq + 1, strlen(eq));
            output[strcspn(output, "\n")] = 0;
        }
        pclose(p);
    }
}

/**
 * @brief Probes CPU Cache hierarchy details (Assignment Question 3). [cite: 133]
 * @param log_fp Pointer to the hardware_info.txt file.
 */
void probe_cache_info(FILE *log_fp) {
    fprintf(log_fp, "\n[Part 2: Question 3 - Cache Hierarchy]\n");
    printf("\nProbing CPU Cache...\n");
    for (int i = 0; i < 4; i++) {
        char path[256];
        sprintf(path, "/sys/devices/system/cpu/cpu0/cache/index%d/size", i);
        FILE *fp = fopen(path, "r");
        if (fp) {
            fclose(fp);
            sprintf(path, "/sys/devices/system/cpu/cpu0/cache/index%d/level", i);
            log_sys_value(path, "Cache Level", log_fp);
            sprintf(path, "/sys/devices/system/cpu/cpu0/cache/index%d/type", i);
            log_sys_value(path, "Type", log_fp);
            sprintf(path, "/sys/devices/system/cpu/cpu0/cache/index%d/size", i);
            log_sys_value(path, "Size", log_fp);
            fprintf(log_fp, "------------------\n");
        }
    }
}

/**
 * @brief Scans for connected USB devices (Assignment Question 2). [cite: 132]
 * @param log_fp Pointer to the hardware_info.txt file.
 */
void scan_usb_devices(FILE *log_fp) {
    fprintf(log_fp, "\n[Part 3: Question 2 - External USB Devices]\n");
    printf("\nScanning USB Bus...\n");
    struct dirent *de;
    DIR *dr = opendir("/sys/bus/usb/devices/");
    if (!dr) return;

    while ((de = readdir(dr)) != NULL) {
        if (de->d_name[0] != '.') {
            char v_path[512], p_path[512];
            sprintf(v_path, "/sys/bus/usb/devices/%s/idVendor", de->d_name);
            sprintf(p_path, "/sys/bus/usb/devices/%s/idProduct", de->d_name);
            FILE *vf = fopen(v_path, "r"), *pf = fopen(p_path, "r");
            if (vf && pf) {
                char vid[16], pid[16];
                fgets(vid, sizeof(vid), vf); fgets(pid, sizeof(pid), pf);
                vid[strcspn(vid, "\n")] = 0; pid[strcspn(pid, "\n")] = 0;
                fprintf(log_fp, "USB Device [%s]: ID %s:%s\n", de->d_name, vid, pid);
                printf("USB Device [%s]: ID %s:%s\n", de->d_name, vid, pid);
                fclose(vf); fclose(pf);
            }
        }
    }
    closedir(dr);
}

/**
 * @brief Generates a static report of the SoC and Memory specifications. [cite: 131, 137]
 * @note Answers Assignment Questions 1, 2, 3, and 7. [cite: 169]
 */
void generate_info_report() {
    FILE *fp = fopen("hardware_info.txt", "w");
    if (!fp) return;

    fprintf(fp, "Hardware Specification Report\n========================================\n");
    char buffer[256];

    FILE *p = popen("grep 'Model' /proc/cpuinfo | cut -d ':' -f 2", "r");
    if (p && fgets(buffer, sizeof(buffer), p)) {
        fprintf(fp, "Target Board : %s", buffer + 1);
        printf("Target Board : %s", buffer + 1);
    }
    pclose(p);

    p = popen("grep 'MemTotal' /proc/meminfo | awk '{print $2, $3}'", "r");
    if (p && fgets(buffer, sizeof(buffer), p)) {
        fprintf(fp, "Total RAM    : %s\n", buffer);
        printf("Total RAM    : %s\n", buffer);
    }
    pclose(p);

    probe_cache_info(fp);
    scan_usb_devices(fp);
    fclose(fp);
    printf("\n[Success] Static info saved to hardware_info.txt\n");
}

/**
 * @brief Runs a stress test and logs thermal/clock data to a file. [cite: 158]
 * @details Performs high-intensity memory copies and logs data every second. [cite: 160]
 * @param duration_sec How long the stress test should run.
 * @note Answers Assignment Questions 27 and 28. [cite: 159]
 */
void run_stress_benchmark(int duration_sec, int num_threads) {
    FILE *fp = fopen("hardware_benchmark.txt", "w");
    if (!fp) return;

    fprintf(fp, "Stress Test (Duration: %ds, Threads: %d)\n", duration_sec, num_threads);
    fprintf(fp, "Time(s),Temp(C),CPU_Freq(MHz),Volts(V)\n");

    printf("Starting stress test with %d threads for %d seconds...\n", num_threads, duration_sec);

    pthread_t threads[num_threads];
    thread_args_t t_args = {duration_sec, 10 * 1024 * 1024};

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, stress_worker, &t_args);
    }

    time_t start = time(NULL);
    int elapsed = 0;
    while (elapsed < duration_sec) {
        time_t now = time(NULL);
        if ((int)(now - start) > elapsed) {
            elapsed = (int)(now - start);
            char temp[32], cpu_f[32], volt[32];
            get_vcgen_data("measure_temp", temp, 32);
            get_vcgen_data("measure_clock arm", cpu_f, 32);
            get_vcgen_data("measure_volts core", volt, 32);

            // 儲存為 CSV 格式
            fprintf(fp, "%d,%s,%ld,%s\n", elapsed, temp, atol(cpu_f)/1000000, volt);
            printf("Elapsed: %d/%ds | Temp: %s | CPU: %ldMHz\n", elapsed, duration_sec, temp, atol(cpu_f)/1000000);
        }
        usleep(100000);
    }

    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
    fclose(fp);
}

/**
 * @brief Main entry point for the exploration tool.
 */
int main() {
    printf("Starting Benchmark Tool...\n");

    int b_time = read_config_int("benchmark_time", 60);
    int num_threads = read_config_int("thread", 1);

    printf("Configuration: Time=%ds, Threads=%d\n", b_time, num_threads);

    generate_info_report();
    run_stress_benchmark(b_time, num_threads);

    printf("\n[Done] Please remember to use 'sudo halt' before unplugging.\n");
    return 0;
}