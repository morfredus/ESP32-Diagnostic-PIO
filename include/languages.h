#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <Arduino.h>
#include <pgmspace.h>

enum Language {
  LANG_EN = 0,
  LANG_FR = 1
};

extern Language currentLanguage;

class TextField {
 public:
  constexpr TextField() : enValue(nullptr), frValue(nullptr) {}
  constexpr TextField(const __FlashStringHelper* en, const __FlashStringHelper* fr)
      : enValue(en), frValue(fr) {}

  operator const __FlashStringHelper*() const { return get(); }

  String str() const {
    const __FlashStringHelper* raw = get();
    return raw ? String(raw) : String();
  }

  const __FlashStringHelper* get(Language lang) const {
    if (lang == LANG_FR && frValue != nullptr) {
      return frValue;
    }
    return enValue;
  }

  const __FlashStringHelper* get() const { return get(currentLanguage); }

 private:
  const __FlashStringHelper* enValue;
  const __FlashStringHelper* frValue;
};

#define TEXT_RESOURCE_MAP(X)\
  X(title, PROJECT_NAME, PROJECT_NAME) \
  X(version, "v", "v") \
  X(access, "Access", "Accès") \
  X(or_text, "or", "ou") \
  X(nav_overview, "Overview", "Vue d'ensemble") \
  X(nav_display_signal, "Display & Signal", "Affichage & Signal") \
  X(nav_sensors, "Sensors", "Capteurs") \
  X(nav_hardware_tests, "Hardware Tests", "Tests Matériel") \
  X(nav_leds, "LEDs", "LEDs") \
  X(nav_screens, "Screens", "Écrans") \
  X(nav_tests, "Advanced Tests", "Tests Avancés") \
  X(nav_gpio, "GPIO", "GPIO") \
  X(nav_wireless, "Wireless", "Sans fil") \
  X(nav_benchmark, "Performance", "Performances") \
  X(nav_export, "Export", "Export") \
  X(nav_select_label, "Select a section", "Sélectionner une section") \
  X(chip_info, "Detailed Chip Information", "Informations détaillées sur la puce") \
  X(full_model, "Full Model", "Modèle complet") \
  X(cpu_cores, "CPU Cores", "Cœurs CPU") \
  X(mac_wifi, "WiFi MAC Address", "Adresse MAC WiFi") \
  X(last_reset, "Last Reset Reason", "Raison du dernier reset") \
  X(chip_features, "Chip Features", "Fonctionnalités de la puce") \
  X(sdk_version, "SDK Version", "Version du SDK") \
  X(idf_version, "ESP-IDF Version", "Version d'ESP-IDF") \
  X(uptime, "Uptime", "Uptime") \
  X(cpu_temp, "CPU Temperature", "Température du CPU") \
  X(memory_details, "Detailed Memory", "Mémoire détaillée") \
  X(flash_memory, "Flash Memory", "Mémoire flash") \
  X(real_size, "Actual Size (board)", "Taille réelle (carte)") \
  X(configured_ide, "Configured (IDE)", "Configurée (IDE)") \
  X(total_size, "Total Size", "Taille Totale") \
  X(used_size, "Used", "Utilisée") \
  X(total_ram, "Total RAM", "RAM totale (SRAM+PSRAM)") \
  X(free_ram, "Free", "RAM libre") \
  X(used_ram, "Used", "RAM utilisée") \
  X(largest_block, "Largest Block", "Plus grand bloc") \
  X(total_psram, "Total PSRAM", "PSRAM totale") \
  X(free_psram, "Free", "PSRAM libre") \
  X(used_psram, "Used", "PSRAM utilisée") \
  X(psram_unavailable, "PSRAM unavailable", "PSRAM indisponible") \
  X(detailed_memory, "Detailed Memory", "Mémoire détaillée") \
  X(allocation_method, "Allocation Method", "Méthode d'allocation") \
  X(internal, "Internal", "Interne") \
  X(internal_dma, "Internal (DMA-capable)", "Interne (compatible DMA)") \
  X(spiram, "SPIRAM", "SPIRAM") \
  X(default_caps, "Default", "Par défaut") \
  X(partition_list, "Partition List", "Liste des partitions") \
  X(label, "Label", "Libellé") \
  X(type, "Type", "Type") \
  X(subtype, "Subtype", "Sous-type") \
  X(address, "Address", "Adresse") \
  X(size, "Size", "Taille") \
  X(app, "app", "app") \
  X(data, "data", "data") \
  X(factory, "factory", "factory") \
  X(nvs, "nvs", "nvs") \
  X(spiffs, "spiffs", "spiffs") \
  X(nvs_keys, "nvs_keys", "nvs_keys") \
  X(efuse, "efuse", "efuse") \
  X(esphttpd, "esphttpd", "esphttpd") \
  X(fat, "fat", "fat") \
  X(ota, "ota", "ota") \
  X(phy, "phy", "phy") \
  X(coredump, "coredump", "coredump") \
  X(unknown, "unknown", "Inconnu") \
  X(wifi_module, "WiFi Module", "Module WiFi") \
  X(status, "Status", "Statut") \
  X(connected, "Connected", "Connecté") \
  X(disconnected, "Disconnected", "Déconnecté") \
  X(ssid, "SSID", "SSID") \
  X(ip_address, "IP Address", "Adresse IP") \
  X(signal_strength, "Signal Strength", "Puissance du signal") \
  X(channel, "Channel", "Canal") \
  X(quality, "Quality", "Qualité") \
  X(excellent, "Excellent", "Excellent") \
  X(good, "Good", "Bon") \
  X(average, "Average", "Moyen") \
  X(weak, "Weak", "Faible") \
  X(very_weak, "Very weak", "Très faible") \
  X(mac_address, "MAC Address", "Adresse MAC") \
  X(hostname, "Hostname", "Nom d'hôte") \
  X(peripherals_detected, "Detected Peripherals", "Périphériques détectés") \
  X(no_peripherals, "No peripherals detected", "Aucun périphérique détecté") \
  X(builtin_led, "Built-in LED", "LED intégrée") \
  X(builtin_led_desc, "Control the onboard LED of your ESP32 module", "Contrôlez la LED embarquée et vérifiez sa réponse.") \
  X(available, "Available", "Disponible") \
  X(not_available, "Not available", "Indisponible") \
  X(test, "Test", "Test") \
  X(turn_on, "Turn ON", "Allumer") \
  X(turn_off, "Turn OFF", "Éteindre") \
  X(blink, "Blink", "Clignotement") \
  X(ws2812b_led, "WS2812B LED Strip", "Ruban LED WS2812B") \
  X(neopixel_desc, "Control an external addressable RGB LED strip (WS2812B/NeoPixel)", "Pilotez vos bandes NeoPixel et testez les animations.") \
  X(pin, "Pin", "Broche") \
  X(led_count, "LED Count", "Nombre de LEDs") \
  X(configure, "Configure", "Configurer") \
  X(test_colors, "Test Colors", "Tester les couleurs") \
  X(rainbow, "Rainbow", "Arc-en-ciel") \
  X(clear, "Clear", "Effacer") \
  X(oled_display, "OLED Display", "Écran OLED") \
  X(label_sda, "SDA Pin", "SDA") \
  X(label_scl, "SCL Pin", "SCL") \
  X(oled_desc, "Configure and test an I2C OLED screen (SSD1306 128x64)", "Configurer et tester un écran OLED I2C (SSD1306 128x64)") \
  X(changes_pins, "Modify I2C pins and test the OLED screen", "Modifie les pins I2C et relance la détection") \
  X(test_display, "Test Display", "Tester l'affichage") \
  X(rgb_led, "RGB LED", "LED RGB") \
  X(rgb_led_desc, "Test an RGB LED with separate red, green, and blue pins", "Test de la LED RGB (3 canaux R, G, B)") \
  X(pin_red, "Red Pin", "Broche rouge") \
  X(pin_green, "Green Pin", "Broche verte") \
  X(pin_blue, "Blue Pin", "Broche bleue") \
  X(test_red, "Test Red", "Tester le rouge") \
  X(test_green, "Test Green", "Tester le vert") \
  X(test_blue, "Test Blue", "Tester le bleu") \
  X(test_white, "Test White", "Tester le blanc") \
  X(test_cycle, "Cycle Colors", "Cycle de couleurs") \
  X(buzzer, "Buzzer", "Buzzer") \
  X(buzzer_desc, "Test an active or passive buzzer", "Test du buzzer avec différentes fréquences") \
  X(beep, "Beep", "Bip") \
  X(melody, "Melody", "Mélodie") \
  X(sensors_intro, "Connect sensors to test them (DHT11/DHT22, photoresistor, HC-SR04, PIR)", "Configurez et testez les capteurs connectés à l'ESP32.") \
  X(dht_sensor, "DHT Sensor", "Capteur DHT") \
  X(dht_sensor_desc, "Digital temperature and humidity sensor", "Capteur numérique de température et d'humidité") \
  X(read_sensor, "Read Sensor", "Lire le capteur") \
  X(temperature, "Temperature", "Température") \
  X(humidity, "Humidity", "Humidité") \
  X(light_sensor, "Photoresistor (Light Sensor)", "Capteur de luminosité") \
  X(light_sensor_desc, "Analog light intensity sensor", "Capteur de luminosité analogique") \
  X(light_value, "Light Value", "Valeur lumineuse") \
  X(distance_sensor, "HC-SR04 (Ultrasonic Distance)", "Capteur de distance") \
  X(distance_sensor_desc, "Ultrasonic distance measurement sensor", "Capteur ultrason HC-SR04") \
  X(trigger_pin, "Trigger Pin", "Broche Trigger") \
  X(echo_pin, "Echo Pin", "Broche Echo") \
  X(measure_distance, "Measure Distance", "Mesurer la distance") \
  X(distance, "Distance", "Distance") \
  X(motion_sensor, "PIR (Motion Detector)", "Capteur de présence") \
  X(motion_sensor_desc, "Passive infrared motion detection sensor", "Détecteur de mouvement PIR") \
  X(check_motion, "Check Motion", "Détecter le mouvement") \
  X(motion_detected, "Motion detected", "Mouvement détecté") \
  X(no_motion, "No motion", "Aucun mouvement") \
  X(display_signal_intro, "Prepare visual and audio outputs for various tests and signals", "Préparez les sorties visuelles et sonores pour vos diagnostics.") \
  X(gpio_desc, "Test any GPIO pin as output or input", "Teste automatiquement l'ensemble des broches GPIO.") \
  X(gpio, "GPIO", "GPIO") \
  X(gpio_test, "GPIO Test", "Test GPIO") \
  X(gpio_pin, "Pin", "Broche GPIO") \
  X(mode, "Mode", "Mode") \
  X(output, "Output", "Sortie") \
  X(input, "Input", "Entrée") \
  X(input_pullup, "Input (PULL-UP)", "Entrée (PULL-UP)") \
  X(input_pulldown, "Input (PULL-DOWN)", "Entrée (PULL-DOWN)") \
  X(action, "Action", "Action") \
  X(set_high, "Set HIGH", "Forcer HIGH") \
  X(set_low, "Set LOW", "Forcer LOW") \
  X(pulse, "Pulse", "Impulsion") \
  X(read_state, "Read State", "Lire l'état") \
  X(high, "HIGH", "HAUT") \
  X(low, "LOW", "BAS") \
  X(adc_test, "ADC Test (Analog Reading)", "Test ADC") \
  X(adc_pin, "ADC Pin", "Broche ADC") \
  X(read_adc, "Read ADC", "Lire l'ADC") \
  X(raw_value, "Raw Value", "Valeur brute") \
  X(voltage, "Voltage", "Tension") \
  X(wifi_scan, "WiFi Network Scan", "Scan WiFi") \
  X(wireless_intro, "Scan nearby WiFi networks", "Analysez les réseaux WiFi.") \
  X(scan_networks, "Scan Networks", "Scanner les réseaux WiFi") \
  X(no_networks, "No networks found", "Aucun réseau trouvé") \
  X(wifi_scan_in_progress, "WiFi scan in progress...", "Recherche des réseaux WiFi") \
  X(wifi_networks_found, "{count} networks found", "{count} réseaux trouvés") \
  X(encryption, "Encryption", "Chiffrement") \
  X(i2c_scan, "I2C Device Scan", "Scan I2C") \
  X(scan_i2c, "Scan I2C Bus", "Scanner le bus I2C") \
  X(no_i2c_devices, "No I2C devices found", "Aucun périphérique I2C") \
  X(i2c_address, "I2C Address", "Adresse I2C") \
  X(wifi_channel, "Channel", "Canal WiFi") \
  X(cpu_benchmark, "CPU Benchmark", "Benchmark CPU") \
  X(cpu_perf_score, "CPU Performance Score", "Score de performance CPU") \
  X(memory_benchmark, "Memory Benchmark", "Benchmark mémoire") \
  X(memory_bandwidth, "Memory Bandwidth", "Bande passante mémoire") \
  X(run_benchmarks, "Run Benchmarks", "Lancer les benchmarks") \
  X(iterations_label, "Iterations", "Itérations") \
  X(allocations_label, "Allocations", "Allocations") \
  X(cpu_time, "CPU Time", "Temps CPU") \
  X(memory_time, "Memory Time", "Temps mémoire") \
  X(gpio_stress, "GPIO Stress Test", "Stress test GPIO") \
  X(start_stress, "Start Stress Test", "Lancer le stress test") \
  X(stop_stress, "Stop Test", "Arrêter le stress test") \
  X(test_duration, "Test Duration", "Durée du test") \
  X(test_in_progress, "Test in progress...", "Test en cours...") \
  X(gpio_test_complete, "GPIO test completed", "Terminé - {count} GPIO testés") \
  X(stress_running, "Stress test running...", "⚠️ Test en cours... Patientez") \
  X(export_intro, "Export diagnostic reports in various formats", "Téléchargez les rapports de diagnostic dans le format souhaité.") \
  X(txt_file, "TXT File", "Fichier TXT") \
  X(json_file, "JSON File", "Fichier JSON") \
  X(csv_file, "CSV File", "Fichier CSV") \
  X(readable_report, "Human-readable diagnostic report", "Rapport texte lisible") \
  X(structured_format, "Structured data format for automation", "Format structuré") \
  X(for_excel, "Tabular format for Excel/spreadsheets", "Pour Excel") \
  X(download_txt, "Download TXT", "Télécharger TXT") \
  X(download_json, "Download JSON", "Télécharger JSON") \
  X(download_csv, "Download CSV", "Télécharger CSV") \
  X(printable_version, "Printable Version", "Version imprimable") \
  X(pdf_format, "PDF-optimized format for printing", "Format PDF") \
  X(open, "Open", "Ouvrir") \
  X(language_label, "Language", "Langue") \
  X(language_switch_error, "Language change failed", "Changement de langue impossible") \
  X(interface_loaded, "Interface loaded", "Interface chargée") \
  X(error_loading, "Error loading data", "Erreur de chargement") \
  X(loading, "Loading...", "Chargement...") \
  X(refresh, "Refresh", "Actualiser") \
  X(save, "Save", "Enregistrer") \
  X(cancel, "Cancel", "Annuler") \
  X(apply, "Apply", "Appliquer") \
  X(name, "Name", "Nom") \
  X(value, "Value", "Valeur") \
  X(description, "Description", "Description") \
  X(enabled, "Enabled", "Activée") \
  X(disabled, "Disabled", "Désactivée") \
  X(success, "Success", "Succès") \
  X(error, "Error", "Erreur") \
  X(warning, "Warning", "Attention") \
  X(info, "Info", "Info") \
  X(close, "Close", "Fermer") \
  X(led_strip, "LED Strip", "Ruban LED") \
  X(oled_screen, "OLED Screen", "Écran OLED 0.96\"") \
  X(detected, "Detected", "Détecté") \
  X(not_detected, "Not detected", "Non détectée") \
  X(i2c_devices, "I2C Devices", "Périphériques I2C") \
  X(gpio_available, "Available GPIO", "GPIO disponibles") \
  X(apply_config, "Apply Configuration", "Appliquer la configuration") \
  X(pin_configuration, "Pin Configuration", "Configuration des pins") \
  X(refresh_data, "Refresh", "Actualiser") \
  X(board, "Board", "Carte") \
  X(brownout, "Brownout", "Brownout") \
  X(category, "Category", "Catégorie") \
  X(channels, "Channels", "canaux") \
  X(chase, "Chase", "Chenillard") \
  X(config, "Config", "Configuration") \
  X(configuration_invalid, "Invalid configuration", "Configuration invalide") \
  X(connected_ssid, "Connected SSID", "SSID connecté") \
  X(cores, "Cores", "cœurs") \
  X(critical, "Critical", "Critique") \
  X(days, "days", "jours") \
  X(deepsleep_exit, "Deep sleep exit", "Sortie de deep sleep") \
  X(detected_active, "Detected (active)", "Détectée et active") \
  X(device_count, "Device count", "Nombre de périphériques") \
  X(devices, "Devices", "périphériques") \
  X(dns, "DNS", "DNS") \
  X(enable_psram_hint, "Enable PSRAM in the IDE settings", "Activez la PSRAM %TYPE% dans l'IDE Arduino (Outils → PSRAM).") \
  X(error_label, "Error", "Erreur") \
  X(export_after_boot, "Export after boot", "secondes après démarrage") \
  X(export_generated, "Export generated", "Rapport généré le") \
  X(fade, "Fade", "Fondu") \
  X(fail, "Fail", "FAIL") \
  X(flash_type, "Flash Type", "Type de flash") \
  X(free, "Free", "Libre") \
  X(frequency, "Frequency", "Fréquence") \
  X(gateway, "Gateway", "Passerelle") \
  X(gpio_interfaces, "GPIO Interfaces", "GPIO et interfaces") \
  X(gpio_invalid, "Invalid GPIO", "GPIO invalide") \
  X(gpio_list, "GPIO List", "Liste des GPIO") \
  X(hours, "hours", "heures") \
  X(i2c_peripherals, "I2C Peripherals", "Périphériques I2C") \
  X(ide_config, "IDE Configuration", "Configuration IDE") \
  X(internal_sram, "Internal SRAM", "SRAM interne") \
  X(max_alloc, "Max Allocation", "Allocation max") \
  X(memory_fragmentation, "Memory Fragmentation", "Fragmentation mémoire") \
  X(memory_status, "Memory Status", "État de la mémoire") \
  X(memory_stress, "Memory Stress", "Stress test mémoire") \
  X(message_displayed, "Message displayed", "Message affiché") \
  X(minutes, "minutes", "minutes") \
  X(model, "Model", "Modèle") \
  X(neopixel, "NeoPixel", "NeoPixel") \
  X(none, "None", "Aucune") \
  X(not_tested, "Not tested", "Non testé") \
  X(off, "OFF", "Arrêt") \
  X(ok, "OK", "OK") \
  X(oled_step_big_text, "Display large text", "Texte grand format") \
  X(oled_step_diagonals, "Draw diagonals", "Lignes diagonales") \
  X(oled_step_executed_prefix, "Step executed:", "Étape exécutée :") \
  X(oled_step_final_message, "OLED test complete", "Message final") \
  X(oled_step_horizontal_lines, "Draw horizontal lines", "Lignes horizontales") \
  X(oled_step_moving_square, "Animate moving square", "Carré en mouvement") \
  X(oled_step_progress_bar, "Render progress bar", "Barre de progression") \
  X(oled_step_scroll_text, "Scroll text", "Texte défilant") \
  X(oled_step_shapes, "Draw shapes", "Formes géométriques") \
  X(oled_step_text_sizes, "Show text sizes", "Tailles de texte") \
  X(oled_step_unavailable, "OLED test unavailable", "OLED non disponible") \
  X(oled_step_unknown, "Unknown OLED step", "Étape inconnue") \
  X(oled_step_welcome, "OLED diagnostic sequence", "Accueil") \
  X(other, "Other", "Autre") \
  X(parameter, "Parameter", "Paramètre") \
  X(performance_bench, "Performance Bench", "Benchmarks de performance") \
  X(pins, "Pins", "broches") \
  X(poweron, "Power-on", "Mise sous tension") \
  X(psram_external, "External PSRAM", "PSRAM (mémoire externe)") \
  X(pwm_test, "PWM Test", "Test PWM") \
  X(pwm_test_desc, "Generate a duty-cycle sweep on the default PWM pin.", "Tester la génération PWM sur les broches choisies") \
  X(start_pwm_test, "Run PWM Test", "Lancer le test PWM") \
  X(revision, "Revision", "Révision") \
  X(signal_power, "Signal power", "Puissance du signal (RSSI)") \
  X(signal_quality, "Signal quality", "Qualité du signal") \
  X(software_reset, "Software reset", "Reset logiciel") \
  X(spi_bus, "SPI Bus", "Bus SPI") \
  X(subnet_mask, "Subnet Mask", "Masque de sous-réseau") \
  X(supported_not_enabled, "Supported but not enabled", "Support détecté (désactivée dans l'IDE)") \
  X(total_gpio, "Total GPIO", "Nombre de GPIO") \
  X(used, "Used", "Utilisée") \
  X(very_good, "Very good", "Très bon") \
  X(wifi_connection, "WiFi connection", "Connexion WiFi") \
  X(wifi_open_auth, "Open authentication", "Ouvert") \
  X(adc_desc, "Analog input diagnostics", "Contrôle les entrées analogiques pour valider l'ADC.") \
  X(animations, "Animations", "Animations") \
  X(apply_color, "Apply Color", "Appliquer la couleur") \
  X(apply_redetect, "Apply & Redetect", "Appliquer et relancer la détection") \
  X(benchmark_desc, "Run CPU and memory benchmarks", "Mesure les performances CPU et mémoire du module.") \
  X(blue, "Blue", "Bleu") \
  X(buzzer_pin, "Buzzer Pin", "Pin du buzzer") \
  X(check_wiring, "Check wiring", "Vérifiez le câblage et les pins I2C") \
  X(click_to_scan, "Click to scan", "Cliquez pour scanner") \
  X(click_to_test, "Click to test", "Cliquez pour tester") \
  X(spi_scan, "SPI Bus Scan", "Scan SPI") \
  X(spi_scan_desc, "List the SPI controllers detected for the active chip.", "Inspecter le bus SPI pour détecter des périphériques") \
  X(start_spi_scan, "Run SPI Scan", "Lancer le scan SPI") \
  X(configure_led_pin, "Configure LED Pin", "Configurer la broche LED") \
  X(configure_neopixel, "Configure NeoPixel", "Configurer le NeoPixel") \
  X(custom_color, "Custom Color", "Couleur personnalisée") \
  X(custom_message, "Custom Message", "Message personnalisé") \
  X(data_export, "Data Export", "Export des données") \
  X(detected_addresses, "Detected Addresses", "Adresses détectées") \
  X(dht_sensor_pin, "DHT Sensor Pin", "Broche du capteur DHT") \
  X(dht_sensor_type, "Sensor type", "Type de capteur DHT") \
  X(dht11_option, "DHT11", "DHT11") \
  X(dht22_option, "DHT22", "DHT22") \
  X(distance_pins, "Distance Sensor Pins", "Pins (Trig, Echo)") \
  X(flash_speed, "Flash Speed", "Vitesse de flash") \
  X(full_test, "Full Test", "Test complet") \
  X(gpio_warning, "A FAIL result can stem from wiring, power, or configuration issues—double-check the setup before declaring the pin faulty.", "Un résultat FAIL peut venir du câblage, de l'alimentation ou d'une mauvaise configuration : validez l'environnement avant d'incriminer la broche.") \
  X(green, "Green", "Vert") \
  X(i2c_pins, "I2C Pins", "Pins I2C") \
  X(ip_unavailable, "IP unavailable", "IP indisponible") \
  X(label_echo, "Echo Pin", "Echo") \
  X(label_trig, "Trigger Pin", "Trig") \
  X(light_level, "Light Level", "Niveau de lumière") \
  X(light_sensor_pin, "Light Sensor Pin", "Pin du capteur de luminosité") \
  X(motion_sensor_pin, "Motion Sensor Pin", "Pin du capteur de présence") \
  X(no_detected, "No devices detected", "Aucun écran OLED détecté. Vérifiez le câblage et les pins I2C ci-dessus.") \
  X(red, "Red", "Rouge") \
  X(rgb_led_pins, "RGB LED Pins", "Pins RGB (R, G, B)") \
  X(rotation, "Rotation", "Rotation") \
  X(sensors_section, "Sensors", "Tests des capteurs") \
  X(show_message, "Show Message", "Afficher le message") \
  X(start_adc_test, "Start ADC Test", "Lancer le test ADC") \
  X(stress_desc, "Run stress tests on the device", "Pousse la mémoire pour détecter les instabilités.") \
  X(stress_warning, "Stress test may reboot the device", "Peut ralentir l'ESP32 temporairement") \
  X(stress_duration, "Stress duration", "Durée du stress test") \
  X(stress_completed, "Stress test completed", "Stress test terminé") \
  X(test_all_gpio, "Test all GPIO", "Tester tous les GPIO") \
  X(test_buzzer, "Test Buzzer", "Tester le buzzer") \
  X(test_dht_sensor, "Test DHT Sensor", "Tester le capteur DHT") \
  X(test_distance_sensor, "Test Distance Sensor", "Mesurer la distance") \
  X(test_light_sensor, "Test Light Sensor", "Mesurer la luminosité") \
  X(test_motion_sensor, "Test Motion Sensor", "Tester la présence") \
  X(test_rgb_led, "Test RGB LED", "Tester la LED RGB") \
  X(turn_off_all, "Turn off all", "Tout éteindre") \
  X(updating, "Updating...", "Mise à jour...") \
  X(white, "White", "Blanc") \
  X(wifi_desc, "Scan nearby WiFi networks", "Analyse les réseaux WiFi et leurs paramètres radio.") \
  X(wifi_scanner, "WiFi Scanner", "Scanner WiFi") \
  X(tft_screen, "TFT Display", "Écran TFT") \
  X(tft_step_boot, "Boot splash", "Écran de démarrage") \
  X(tft_step_colors, "Color test", "Test des couleurs") \
  X(tft_step_shapes, "Shapes", "Formes géométriques") \
  X(tft_step_text, "Text rendering", "Rendu de texte") \
  X(tft_step_lines, "Lines patterns", "Motifs de lignes") \
  X(tft_step_animation, "Animation", "Animation") \
  X(tft_step_progress, "Progress bar", "Barre de progression") \
  X(tft_step_final, "Final message", "Message final") \
  X(boot_screen, "Boot Screen", "Écran de démarrage") \
  X(resolution, "Resolution", "Résolution") \
  X(spi_pins, "SPI Pins", "Pins SPI") \
  X(tft_test_running, "TFT test in progress...", "Test TFT en cours...") \
  X(tft_step_running, "TFT step executing...", "Étape TFT en cours...") \
  X(restoring_boot_screen, "Restoring boot screen...", "Restauration de l'écran de démarrage...") \
  X(tft_brightness, "Brightness", "Luminosité") \
  X(brightness_level, "Brightness level", "Niveau de luminosité") \
  X(gps_sensor, "GPS Module", "Module GPS") \
  X(gps_sensor_desc, "Global Positioning System receiver (NEO-6M/NEO-8M)", "Récepteur GPS (NEO-6M/NEO-8M)") \
  X(gps_latitude, "Latitude", "Latitude") \
  X(gps_longitude, "Longitude", "Longitude") \
  X(gps_altitude, "Altitude", "Altitude") \
  X(gps_satellites, "Satellites", "Satellites") \
  X(gps_hdop, "HDOP", "HDOP") \
  X(gps_fix_type, "Fix Type", "Type de Fix") \
  X(gps_speed, "Speed", "Vitesse") \
  X(gps_course, "Course", "Cap") \
  X(gps_status, "GPS Status", "Statut GPS") \
  X(test_gps, "Test GPS", "Tester le GPS") \
  X(refresh_gps, "Refresh GPS", "Rafraîchir GPS") \
  X(gps_module, "GPS Module", "Module GPS") \
  X(gps_module_desc, "NEO-6M/NEO-8M GPS Module (UART)", "Module GPS NEO-6M/NEO-8M (UART)") \
  X(aht20_sensor, "AHT20 Sensor", "Capteur AHT20") \
  X(aht20_sensor_desc, "Temperature and humidity sensor (I2C)", "Capteur de température et humidité (I2C)") \
  X(bmp280_sensor, "BMP280 Sensor", "Capteur BMP280") \
  X(bmp280_sensor_desc, "Pressure and temperature sensor (I2C)", "Capteur de pression et température (I2C)") \
  X(environmental_sensors, "Environmental Sensors", "Capteurs environnementaux") \
  X(environmental_sensors_desc, "AHT20 (temperature/humidity) + BMP280 (pressure)", "AHT20 (temp/humidité) + BMP280 (pression)") \
  X(refresh_env_sensors, "Refresh Sensors", "Rafraîchir capteurs") \
  X(test_env_sensors, "Test Sensors", "Tester capteurs") \
  X(pressure, "Pressure", "Pression") \
  X(pressure_hpa, "Pressure (hPa)", "Pression (hPa)") \
  X(temperature_avg, "Average Temperature", "Température moyenne") \
  X(altitude_calculated, "Calculated Altitude", "Altitude calculée") \
  X(test_environmental, "Test Sensors", "Tester les capteurs") \
  X(knots, "knots", "nœuds") \
  X(hpa, "hPa", "hPa") \
  X(nav_input_devices, "Input Devices", "Périphériques") \
  X(nav_memory, "Memory", "Mémoire") \
  X(sd_card, "SD Card Reader", "Lecteur carte SD") \
  X(sd_card_desc, "SD Card reader for data storage and logging", "Lecteur de carte SD pour stockage de données et logs") \
  X(sd_pins_spi, "SPI Pins", "Pins SPI") \
  X(sd_pin_miso, "MISO", "MISO") \
  X(sd_pin_mosi, "MOSI", "MOSI") \
  X(sd_pin_sclk, "SCLK", "SCLK") \
  X(sd_pin_cs, "CS", "CS") \
  X(test_sd, "Test SD", "Tester SD") \
  X(sd_card_type, "Card Type", "Type de carte") \
  X(sd_card_size, "Card Size", "Taille carte") \
  X(sd_read_speed, "Read Speed", "Vitesse lecture") \
  X(sd_write_speed, "Write Speed", "Vitesse écriture") \
  X(rotary_encoder, "Rotary Encoder HW-040", "Encodeur rotatif HW-040") \
  X(rotary_encoder_desc, "Rotary encoder with integrated push button", "Encodeur rotatif avec bouton intégré") \
  X(rotary_pins, "Encoder Pins", "Pins encodeur") \
  X(rotary_pin_clk, "CLK", "CLK") \
  X(rotary_pin_dt, "DT", "DT") \
  X(rotary_pin_sw, "SW", "SW") \
  X(rotary_position, "Position", "Position") \
  X(rotary_button, "Button", "Bouton") \
  X(rotary_button_released, "Released", "Relâché") \
  X(rotary_button_pressed, "Pressed", "Appuyé") \
  X(test_rotary, "Test", "Tester") \
  X(rotary_monitor, "Monitor", "Surveiller") \
  X(rotary_reset, "Reset", "Réinitialiser") \
  X(sd_test_read, "Read Test", "Test lecture") \
  X(sd_test_write, "Write Test", "Test écriture") \
  X(sd_format, "Format Card", "Formater carte") \
  X(sd_format_warning, "Warning: This will erase all data on the SD card!", "Attention : Ceci effacera toutes les données de la carte SD !") \
  X(sd_format_confirm, "Are you sure you want to format the SD card?", "Êtes-vous sûr de vouloir formater la carte SD ?") \
  X(gpio_shared_warning, "Shared GPIO 13 (TFT + SD – MISO)", "GPIO 13 partagé (TFT + SD – MISO)") \
  X(gpio_13_shared_desc, "GPIO 13 is shared between the TFT and SD Card (MISO line). Strictly compliant SPI wiring and proper software management are required.", "Le GPIO 13 est partagé entre le TFT et la carte SD (ligne MISO). Un câblage SPI strictement conforme et une gestion logicielle adaptée sont requis.") \
  X(sd_read_test_running, "Running read test...", "Test de lecture en cours...") \
  X(sd_write_test_running, "Running write test...", "Test d'écriture en cours...") \
  X(sd_format_running, "Formatting SD card...", "Formatage de la carte SD...") \
  X(memory_section, "Memory & Storage", "Mémoire & Stockage") \
  X(memory_intro, "Detailed memory information and external storage", "Informations mémoire détaillées et stockage externe") \
  X(input_devices_section, "Input Devices & Controls", "Périphériques d'entrée") \
  X(input_devices_intro, "Buttons, encoders and user input devices", "Boutons, encodeurs et dispositifs d'entrée utilisateur") \
  X(button_boot, "Boot Button", "Bouton Boot") \
  X(button_boot_desc, "Built-in boot button for firmware flashing and reset", "Bouton boot intégré pour flashage et réinitialisation") \
  X(button_1, "User Button 1", "Bouton Utilisateur 1") \
  X(button_1_desc, "User programmable button 1 with internal pull-up", "Bouton utilisateur 1 programmable avec pull-up interne") \
  X(button_2, "User Button 2", "Bouton Utilisateur 2") \
  X(button_2_desc, "User programmable button 2 with internal pull-up", "Bouton utilisateur 2 programmable avec pull-up interne") \
  X(button_pin, "Pin", "Pin") \
  X(button_state, "State", "État") \
  X(button_pressed, "Pressed", "Appuyé") \
  X(button_released, "Released", "Relâché") \
  X(monitor_button, "Monitor", "Surveiller") \
  X(stop_monitoring, "Stop", "Arrêter") \
  X(coming_soon, "Coming Soon", "Bientôt disponible")

namespace Texts {
#define DECLARE_TEXT(identifier, en, fr) \
  inline const TextField identifier = TextField(F(en), F(fr));
  TEXT_RESOURCE_MAP(DECLARE_TEXT)
#undef DECLARE_TEXT

  struct ResourceEntry {
    const char* key;
    const TextField* field;
  };

  inline const ResourceEntry* getResourceEntries(size_t& count) {
    static const ResourceEntry entries[] = {
#define REGISTER_TEXT(identifier, en, fr) { #identifier, &identifier },
      TEXT_RESOURCE_MAP(REGISTER_TEXT)
#undef REGISTER_TEXT
    };
    count = sizeof(entries) / sizeof(entries[0]);
    return entries;
  }
}

inline void setLanguage(Language lang) {
  currentLanguage = lang;
}

inline Language getLanguage() {
  return currentLanguage;
}

#ifdef TEXT_RESOURCE_MAP
#undef TEXT_RESOURCE_MAP
#endif

#endif  // LANGUAGES_H
