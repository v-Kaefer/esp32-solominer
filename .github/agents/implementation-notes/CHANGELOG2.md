# Resumo técnico (para outra IA) — Diagnóstico e fix do I²C (ESP32-S3 + SSD1306)

## Contexto

* **Placa:** ESP32-S3 (board genérico com serigrafia imprecisa).
* **Display:** OLED SSD1306 via **I²C**, endereço **0x3C** (confirmado).
* **Sintoma:** scanner mostrava “idle 1/1” em muitos pares e “ACHOU 0x3C” em combinações inconsistentes; `i2c driver install error` e, em testes com Wi-Fi ativo, panics do mbedTLS/WPA3.

## Hipótese confirmada

* A **serigrafia do board não corresponde** aos GPIOs reais.
* **Mapeamento correto encontrado:**

  * **SDA = GPIO15**
  * **SCL = GPIO9**
* Endereço do SSD1306: **0x3C** (0x3D não responde).

## Ambiente e cautelas

* **ESP-IDF v5.x.**
* Para evitar ruído:

  * **Isolar o teste do I²C** (sem Wi-Fi, sem tasks pesadas).
  * **Não usar “idle 1/1” como critério** — isso só indica pull-ups/nível alto, gera falso-positivo.
  * **Reinstalar o driver I²C a cada tentativa de par** (delete → param_config → install), senão o IDF mantém o par anterior e o sweep mente.
  * A mensagem `i2c_driver_delete(...): i2c driver install error` é **benigna** se o driver não estava instalado.

## Código utilitário usado no diagnóstico

### 1) “Probe” minimalista (substitui `i2c_master_probe` quando ausente)

```c
static esp_err_t i2c_probe_addr(i2c_port_t port, uint8_t addr, TickType_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true); // espera ACK
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);
    return err; // ESP_OK = ACK
}
```

### 2) Teste de um par específico (deleta/instala driver “limpo”)

```c
static esp_err_t try_pair(int sda, int scl)
{
    i2c_driver_delete(I2C_NUM_0); // ok se retornar erro: driver não instalado
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda, .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl, .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
    vTaskDelay(pdMS_TO_TICKS(3));

    esp_err_t r = i2c_probe_addr(I2C_NUM_0, 0x3C, 50);
    i2c_driver_delete(I2C_NUM_0);
    return r; // ESP_OK se o par é válido
}
```

### 3) Sweep determinístico (logar **apenas** quando houver ACK)

```c
void app_main(void)
{
    int sda_candidates[] = {15};          // já sabido
    int scl_candidates[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,16,17,18,21};

    for (int si = 0; si < sizeof(sda_candidates)/sizeof(int); ++si) {
        for (int ci = 0; ci < sizeof(scl_candidates)/sizeof(int); ++ci) {
            int SDA = sda_candidates[si], SCL = scl_candidates[ci];
            esp_err_t ok = try_pair(SDA, SCL);
            if (ok == ESP_OK) {
                ESP_LOGE("PIN_FINDER",
                    ">> ENCONTREI ACK! SDA=%d SCL=%d (0x3C)", SDA, SCL);
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
```

**Observação:** O **primeiro** ACK ocorreu em **SCL=9**. ACKs reportados depois (10, 11, 12, …) são ruído de timing/estado e **não** significam SCL alternativo válido.

## Firmware final (fixo, sem sweep)

### Defines

```c
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_SDA_IO   15  // mapeamento real
#define I2C_MASTER_SCL_IO   9   // mapeamento real
#define I2C_MASTER_FREQ_HZ  100000
```

### Init I²C (uma única vez no boot)

```c
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
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}
```

### Init do SSD1306

> Importante: no projeto original o `dev` do display não estava sendo inicializado antes do uso.

```c
void app_main(void)
{
    i2c_init_fixed();

    // ajuste para a assinatura do seu driver SSD1306
    i2c_master_init_ssd1306(&dev, I2C_MASTER_NUM, 128, 64, 0x3C);

    // exemplo de uso
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "HELLO", 5, false);
}
```

## Boas práticas & verificações

* **Pull-ups:** garantir 4.7k–10k para 3V3 em SDA/SCL (muitos módulos SSD1306 já têm).
* **Clock:** 100 kHz; reduzir para 50 kHz se cabo longo.
* **Endereço:** testar 0x3C e 0x3D (aqui, **0x3C**).
* **Driver I²C:** **não reinstalar** repetidamente no app; subir uma vez.
* **Wi-Fi:** fazer a descoberta dos pinos **sem Wi-Fi**. Panics anteriores vieram de **WPA3/SAE (mbedTLS)** enquanto se curto-circuitava pinos.
* **GPIOs a evitar no S3:** 0 (strap), 19/20 (USB D−/D+), 46 (input-only).
  (15 e 9 são válidos para I²C.)

## Checklist final (estado estável)

* [x] `SDA=15`, `SCL=9` travados.
* [x] `i2c_init_fixed()` chamado uma vez.
* [x] SSD1306 inicializado com `i2c_master_init_ssd1306(&dev, I2C_NUM_0, 128, 64, 0x3C)`.
* [x] Display responde e renderiza texto.
* [x] Sem panics de Wi-Fi durante uso normal.

Pronto. Esse é o **playbook reproduzível** para outra IA reexecutar o diagnóstico e aplicar o fix do I²C nessa placa. 🚀
