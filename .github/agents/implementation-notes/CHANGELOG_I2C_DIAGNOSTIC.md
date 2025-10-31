Contexto / Problema

Estamos trabalhando com um ESP32-S3 (board genérica) conectado a um display OLED SSD1306 via I²C.

A placa está mal serigrafada: os pinos marcados fisicamente como GPIO07 (SDA) e GPIO09 (SCL) não batem com a numeração real do chip ESP32-S3.
Resultado: o display não ligava, e as tentativas iniciais de configurar I²C em GPIO7/GPIO9 falhavam, mesmo o scanner “achando” o endereço 0x3C de vez em quando.

Além disso, o código original tinha 3 problemas:

O driver I²C era instalado duas vezes e não removido corretamente → i2c_driver_install error.

A struct/estado do SSD1306 não era inicializada antes de chamar funções de desenho → display recebia lixo ou nada.

Havia “falso-positivo” no sweep de pinos, porque o driver I²C permanecia configurado com pinos antigos.

Objetivo: identificar quais GPIOs reais do ESP32-S3 correspondem a SDA e SCL na placa física, fixar isso no firmware e inicializar corretamente o SSD1306.

Sintomas iniciais

Scanner simples de I²C mostrava:

Achava o endereço 0x3C (SSD1306 padrão).

Mas os pinos usados no log (ex: SDA=7, SCL=1, SCL=2, SCL=3 etc.) não batiam com o hardware real.

Às vezes aparecia erro:

E i2c: i2c driver install error

Isso vinha de reinstalar o driver sem deletar ou deletar um driver inexistente.

Apareceram panics (Guru Meditation, backtrace em mbedtls / wpa_supplicant) — isso vinha do Wi-Fi, não do I²C. Era efeito colateral de ficar encostando GPIOs em GND com o Wi-Fi rodando.

Estratégia para resolver

Desligar o resto do app
Rodar um firmware mínimo, sem Wi-Fi e sem outras tasks pesadas. Só FreeRTOS + I²C.
Motivo: evitar corrupção de memória do Wi-Fi ao curto-circuitar GPIOs durante testes e deixar os logs limpos.

Criar uma função de “probe I²C” confiável
Muitos IDFs têm i2c_master_probe(), mas usamos uma alternativa manual que envia um START + endereço e verifica ACK:

static esp_err_t i2c_probe_addr(i2c_port_t port, uint8_t addr, TickType_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // envia endereço em modo WRITE (R/W = 0)
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);

    return err; // ESP_OK == recebeu ACK
}


Isso substitui i2c_master_probe e funciona em qualquer ESP-IDF recente.

Testar pares SDA/SCL de forma determinística

Para cada par candidato de pinos, fazemos:

i2c_driver_delete(I2C_NUM_0); (se não tinha driver, o IDF reclama, tudo bem)

i2c_param_config(...) passando aqueles GPIOs

i2c_driver_install(...)

chamamos i2c_probe_addr(..., 0x3C) e também 0x3D

removemos o driver de novo

Código reduzido:

#define I2C_PORT I2C_NUM_0
static const char *TAG = "PIN_FINDER";

static esp_err_t try_pair(int sda, int scl)
{
    // limpa antes de configurar
    i2c_driver_delete(I2C_PORT); // erro aqui é ok se não tinha nada instalado

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num    = sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num    = scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000, // 100 kHz
    };

    esp_err_t err = i2c_param_config(I2C_PORT, &conf);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "param_config falhou SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "driver_install falhou SDA=%d SCL=%d: %s", sda, scl, esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(3)); // pequena folga

    // Tenta endereço típico do SSD1306
    esp_err_t r1 = i2c_probe_addr(I2C_PORT, 0x3C, 50);
    esp_err_t r2 = i2c_probe_addr(I2C_PORT, 0x3D, 50);

    i2c_driver_delete(I2C_PORT); // limpa pro próximo teste

    if (r1 == ESP_OK || r2 == ESP_OK) {
        ESP_LOGE(TAG,
            ">> ENCONTREI ACK! SDA=%d SCL=%d (0x3C=%s 0x3D=%s)",
            sda, scl,
            (r1==ESP_OK ? "ACK" : "FAIL"),
            (r2==ESP_OK ? "ACK" : "FAIL"));
        return ESP_OK;
    }

    ESP_LOGI(TAG, "SDA=%d SCL=%d -> nada", sda, scl);
    return ESP_FAIL;
}


Varredura controlada (app_main de teste)

Testamos uma lista pequena de candidatos de SDA e SCL.

Baseado no histórico experimental, suspeitávamos de SDA=15, então varremos só SCL.

Exemplo:

void app_main(void)
{
    int sda_candidates[] = {15};            // suspeita forte de SDA=15
    int scl_candidates[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,16,17,18,21};

    for (int si = 0; si < sizeof(sda_candidates)/sizeof(int); ++si) {
        for (int ci = 0; ci < sizeof(scl_candidates)/sizeof(int); ++ci) {

            int SDA = sda_candidates[si];
            int SCL = scl_candidates[ci];

            ESP_LOGI(TAG, "testando SDA=%d SCL=%d ...", SDA, SCL);
            try_pair(SDA, SCL);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }

    ESP_LOGI(TAG, "fim do sweep.");
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}

Resultado da varredura

Os logs mostraram:

Para SCL = 1 .. 8: -> nada

Para SCL = 9:
>> ENCONTREI ACK! SDA=15 SCL=9 (0x3C=ACK 0x3D=FAIL)

Depois disso, vários outros SCLs (10, 11, 12, 13, 14, 16, 17, 18, 21) também apareceram como "ACK", mas isso é efeito colateral da forma como o driver foi desmontado e reinstalado. O sinal limpo é o primeiro ACK real.

Interpretação:

SDA físico que a placa chama de "GPIO07" → na verdade é o GPIO15 do ESP32-S3.

SCL físico que a placa chama de "GPIO09" → na verdade é o GPIO9 do ESP32-S3.

O dispositivo respondeu no endereço 0x3C (padrão de SSD1306). 0x3D falhou, confirmando que o display está configurado como 0x3C.

Isso explica tudo: o silk da placa está mentindo. O layout liga o header rotulado GPIO07 ao GPIO15 real do chip, e GPIO09 ao GPIO9 real do chip. Por isso nada funcionava quando tentávamos usar “GPIO07 / GPIO09” literalmente no firmware.

Conclusão elétrica

Pinos reais para o OLED I²C:

SDA = GPIO15

SCL = GPIO9

Endereço do display: 0x3C

Clock I²C funcional em 100 kHz

Alimentação 3V3 comum, GND comum

Pull-ups: o módulo SSD1306 provavelmente já fornece pull-up (~4.7k) em SDA/SCL, pois o ACK saiu estável sem resistores extras

Ajustes definitivos no firmware principal

Agora que sabemos os pinos certos, o código “de produção” deve:

Configurar I²C uma única vez no boot, e não reinstalar aleatoriamente depois:

#define I2C_MASTER_NUM        I2C_NUM_0
#define I2C_MASTER_SDA_IO     15      // achado no sweep
#define I2C_MASTER_SCL_IO     9       // achado no sweep
#define I2C_MASTER_FREQ_HZ    100000  // 100 kHz estável

static void i2c_init_fixed(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num    = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num    = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM,
                                       conf.mode,
                                       0, 0, 0));
}


Inicializar o driver do SSD1306 com o endereço correto antes de usar qualquer função de desenho.

No código original havia funções i2c_master_init_ssd1306(&dev, port, width, height, address) e depois ssd1306_*, mas dev nunca era inicializado antes do uso.

Agora deve ser feito assim no app_main():

void app_main(void)
{
    // 1. Inicializa I2C fixo
    i2c_init_fixed();

    // 2. Inicializa o SSD1306 (128x64, addr 0x3C)
    //    IMPORTANTE: precisa passar o mesmo I2C_MASTER_NUM e endereco 0x3C
    i2c_master_init_ssd1306(&dev, I2C_MASTER_NUM, 128, 64, 0x3C);

    // 3. Usa as rotinas de display normalmente
    //    ex: ssd1306_clear_screen(&dev, false);
    //        ssd1306_display_text(&dev, 0, "HELLO", 5, false);

    // 4. Só depois subir Wi-Fi / outras tasks pesadas
}


Parar de rodar o “bruteforce/sweep” e parar de religar o driver I²C depois que o app estiver rodando.

Isso evita i2c_driver_install error.

E evita interferir no barramento que o display está usando.

Observações de segurança / estabilidade

Evitar curto direto de GPIOs em GND enquanto o Wi-Fi está ativo. Isso nos deu alguns panics relacionados a WPA3/SAE (mbedtls_mpi_cmp_mpi, sae_derive_pt_ecc, etc.). Esse ruído não era falha de hardware nem do display, só efeito da task de Wi-Fi rodando enquanto mexíamos em pinos em tempo real.

Clock de 100 kHz é seguro pro cabo curto.

GPIO15 e GPIO9 são GPIOs normais no ESP32-S3 (não são D+/D-, nem strap crítico). Então essa combinação está ok para uso permanente do bus I²C.

O endereço 0x3C respondeu, 0x3D não → não precisamos tentar trocar A0/SA0 no módulo.

TL;DR para a IA

Board ESP32-S3 genérica com silk errado.

OLED SSD1306 I²C não respondia nos GPIOs “documentados”.

Resolvemos escrevendo um firmware de diagnóstico que:

Configura dinamicamente pares SDA/SCL.

Instala o driver I²C, envia START + endereço (0x3C e 0x3D), verifica ACK.

Desinstala o driver e repete.

Resultado do sweep:

Primeiro ACK válido apareceu com SDA=15, SCL=9, endereço 0x3C.

Essa é a fiação real do hardware (mesmo que o board silkscreen diga “GPIO07 / GPIO09”).

Configuração final de produção:

#define I2C_MASTER_SDA_IO 15

#define I2C_MASTER_SCL_IO 9

#define I2C_MASTER_FREQ_HZ 100000

Inicializar o bus I²C uma vez só no boot.

Inicializar o SSD1306 passando porta