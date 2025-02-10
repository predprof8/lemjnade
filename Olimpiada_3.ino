#include <Servo.h>

Servo servo1;

const int recipe_buttons[] = {2, 3, 4, 5, 6, 7};
const int motor_pins[] = {9, 10, 11};
const int end_caps[] = {22, 23, 24, 25};
const int glass_LEDs[] = {30, 31, 32, 33};
const int glass_buttons[] = {40, 41, 42, 43};
const int start_button = 44;
const int complete_LED = 34;
const int rotation_angles[] = {42, 65, 90, 118};
const int pour_durations[] = {1000, 1500, 2000, 2500};
const int max_orders = 4; // Максимальное количество заказов
int angles_for_order[max_orders]; // Массив для хранения углов поворота при заказе
int recipes_for_order[max_orders]; // Массив для хранения рецептов при заказе
int used_glass_buttons[max_orders];  // Массив для хранения нажатых кнопок при заказе
int order_count = 0; // Счетчик заказов
struct Recipe {
  int motor_durations[3];  // Длительность наливания для каждого мотора (в мс)
  int motor_sequence[3];   // Очередность работы моторов (номера моторов)
};
const Recipe recipes[] = {
  {{1050}, {0}}, // Рецепт 1 "Газированая вода": длительность наливания {1050 мс (50 мл)}, моторы {0}
  {{240}, {1}}, // Рецепт 2 "Мятный сироп": длительность наливания {240 мс (10 мл)}, моторы {1}
  {{950}, {2}}, // Рецепт 3 "Апельсиновый сок": длительность наливания {950 мс (40 мл)}, моторы {2}
  {{1700, 480}, {0, 1}}, // Рецепт 4 "Мятный": длительность наливания {1700 мс (80 мл), 480 мс (20 мл)}, моторы {0, 1}
  {{630, 1188}, {0, 2}}, // Рецепт 5 "Заводной апельсин": длительность наливания {630 мс (30 мл), 1188 мс (20 мл)}, моторы {0, 2}
  {{735, 1070, 240}, {0, 2, 1}}, // Рецепт 6 "Тройной": длительность наливания {735 мс (35 мл), 1070 мс (45 мл), 240 мс (10 мл)}, моторы {0, 2, 1}
};

void setup() 
{
  Serial.begin(9600);
  for (int i = 0; i < 3; i++) {
    pinMode(motor_pins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(glass_LEDs[i], OUTPUT);
    pinMode(glass_buttons[i], INPUT);
    pinMode(end_caps[i], INPUT);
  }
  for (int i = 0; i < 6; i++) {
    pinMode(recipe_buttons[i], INPUT);
  }
  pinMode(start_button, INPUT);
  pinMode(complete_LED, OUTPUT);
  servo1.attach(8);
}

void pour_liquid(int angle, int recipe_index)
{
    servo1.write(angle);
    delay(1000); // Задержка между поворотами серво
    Serial.print("Recipe index: ");
    Serial.println(recipe_index);
    Serial.print("Angle: ");
    Serial.println(angle);
    // Получаем рецепт
    Recipe current_recipe = recipes[recipe_index];
    // Запускаем моторы в соответствии с рецептом
    for (int index = 0; index <= 3; index++) {
        Serial.print("Current end cup: ");
        Serial.println(end_caps[used_glass_buttons[index]]);
        int motor_index = current_recipe.motor_sequence[index];
        int motor_duration = current_recipe.motor_durations[index];
        if (motor_duration) { // Проверяем, нужно ли включать мотор
          Serial.print("Motor pin number: ");
          Serial.println(motor_pins[motor_index]);
          Serial.print("Motor duration: ");
          Serial.println(motor_duration);
          digitalWrite(motor_pins[motor_index], HIGH); // Включаем мотор
          delay(motor_duration); // Небольшая задержка для стабильности
          digitalWrite(motor_pins[motor_index], LOW); // Выключаем мотор
        }
        delay(400); // Задержка между сменой моторов
    }
    Serial.println("                    ");
    Serial.println("Pour liquid over");
    Serial.println("                    ");
}

void add_order(int glass_index, int recipe_index)
{
  if (order_count >= max_orders) {
    return;
  }
  digitalWrite(glass_LEDs[glass_index], HIGH);
  angles_for_order[order_count] = rotation_angles[glass_index];
  recipes_for_order[order_count] = recipe_index;
  used_glass_buttons[order_count] = glass_index;
  order_count++;
  Serial.print("order_count: ");
  Serial.println(order_count);
  Serial.print("Order added for Glass ");
  Serial.println(glass_index + 1);
  delay(600);
}

void accept_the_order(int recipe_number)
{
  bool recipe_selected = true;
  while (recipe_selected) {
    if (order_count >= max_orders) {
      break;
    }
    for (int i = 0; i < 4; i++) {
      if (digitalRead(glass_buttons[i]) == HIGH && digitalRead(end_caps[i]) == HIGH) {
        add_order(i, recipe_number);
      }
    }
    for (int i = 0; i < 6; i++) {
      if (digitalRead(recipe_buttons[i]) == HIGH || digitalRead(start_button) == HIGH) {
        recipe_selected = false;
        break;
      }
    }
  }
}

void process_orders()
{
  for (int i = 0; i < 4; i++) {
    digitalWrite(glass_LEDs[i], LOW);
  }
  Serial.println("----------");
  Serial.println("Orders:");
  for (int i = 0; i < max_orders; ++i) {
    Serial.print("Angle ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(angles_for_order[i]);
    Serial.print("Recipe ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(recipes_for_order[i]);
    Serial.print("Used glass buttons ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(used_glass_buttons[i]);
    Serial.println("                    ");
  }
  Serial.println("----------");
  for (int i = 0; i < 4; i++) {
    if (angles_for_order[i] != 0 && digitalRead(end_caps[used_glass_buttons[i]]) == HIGH) {
      Serial.println("Process the order...");
      pour_liquid(angles_for_order[i], recipes_for_order[i]);
    }
  }
  servo1.write(0);
  for (int i = 0; i < max_orders; ++i) {
    angles_for_order[i] = 0;
    recipes_for_order[i] = 0;
  }
  order_count = 0;
  Serial.println("Orders clear");
  delay(1000);
  for (int i = 0; i < 4; ++i) {
    digitalWrite(complete_LED, HIGH);
    delay(1000);
    digitalWrite(complete_LED, LOW);
  }
  Serial.println("          ");
}

void loop() 
{
  servo1.write(0);
  if (digitalRead(start_button) == HIGH) {
    process_orders();
  }
  for (int i = 0; i < 6; i++) {
    if (digitalRead(recipe_buttons[i]) == HIGH) {
      digitalWrite(complete_LED, HIGH);
      delay(500);
      digitalWrite(complete_LED, LOW);
      Serial.println("Accept the order...");
      Serial.print("Selected recipe button ");
      Serial.println(recipe_buttons[i] - 1);
      delay(1000);
      accept_the_order(recipe_buttons[i] - 2);
    }
  }
}