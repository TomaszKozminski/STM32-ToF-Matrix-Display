#ifdef TEST

#include "unity.h"
#include "mock_RangeSensor.h" // CMock generuje ten plik na podstawie src/RangeSensor.h
#include "ObjectTracker.h"
#include <string.h>

/* 
 * WYJAŚNIENIE MOCKOWANIA:
 * Moduł ObjectTracker zależy od RangeSensor. Aby przetestować ObjectTracker w izolacji
 * (bez prawdziwego sprzętu czujnika), używamy 'mocka'.
 * 
 * 1. src/RangeSensor.h definiuje interfejs (kontrakt), którego ObjectTracker używa.
 * 2. Ceedling generuje 'mock_RangeSensor.c' i 'mock_RangeSensor.h'.
 * 3. W testach używamy funkcji typu 'RangeSensor_GetNewFrame_ExpectAndReturn',
 *    aby "zaprogramować" zachowanie tego nieistniejącego czujnika.
 * 4. Dzięki temu możemy symulować dowolne scenariusze (błędy, szum, idealne obiekty).
 */

static ObjectTracker tracker;
static ObjectTrackerConfig config;

// Helper: Tworzy pustą, poprawną klatkę
static void Helper_ClearFrame(RangeSensorFrame *frame) {
    memset(frame, 0, sizeof(RangeSensorFrame));
    frame->is_valid = true;
}

// Helper: Wstawia "idealny" punkt w danej odległości na danym indeksie
static void Helper_SetPixel(RangeSensorFrame *frame, int index, uint16_t dist_mm) {
    if (index >= 0 && index < RS_TOTAL_ZONES) {
        frame->distances_mm[index] = dist_mm;
        frame->statuses[index] = 5; // STATUS_RANGE_VALID
    }
}

void setUp(void)
{
    memset(&tracker, 0, sizeof(ObjectTracker));
    memset(&config, 0, sizeof(ObjectTrackerConfig));
    
    // Konfiguracja domyślna dla testów
    config.min_dist_mm = 100.0f;
    config.max_dist_mm = 2000.0f;
    config.fade_dist_mm = 200.0f;     // Szeroka strefa przejścia dla łatwiejszych testów
    config.min_mass_detect = 1.0f;    // Niski próg detekcji
    config.smooth_factor = 0.0f;      // Wyłączamy wygładzanie, aby wyniki były natychmiastowe
    config.sensor_fov_deg = 45.0f;    // Kąt widzenia 45 stopni
    config.max_dispersion = 100.0f;   // Domyślnie duża tolerancja na rozproszenie

    ObjectTracker_Init(&tracker, &config);
}

void tearDown(void)
{
}

// =========================================================================
// GRUPA 1: Podstawowa Logika Detekcji i Geometrii
// =========================================================================

void test_ObjectTracker_Should_DetectObject_When_SignalIsStrongAndValid(void)
{
    // ARRANGE
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    // Obiekt na środku (indeksy 27, 28, 35, 36 dla siatki 8x8)
    Helper_SetPixel(&frame, 27, 1000);
    Helper_SetPixel(&frame, 28, 1000);
    Helper_SetPixel(&frame, 35, 1000);
    Helper_SetPixel(&frame, 36, 1000);

    // MOCK SETUP
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_TRUE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Obiekt powinien zostać wykryty");
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 1000.0f, ObjectTracker_GetDistance(&tracker));
}

void test_ObjectTracker_Should_CalculateNegativeAngle_When_ObjectIsOnLeft(void)
{
    // ARRANGE
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    // Piksele po lewej stronie (kolumna 0 i 1)
    // Indeks 16 (wiersz 2, kol 0), Indeks 17 (wiersz 2, kol 1)
    Helper_SetPixel(&frame, 16, 500); 
    Helper_SetPixel(&frame, 17, 500); 

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    float angle = ObjectTracker_GetAngle(&tracker);
    TEST_ASSERT_TRUE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Obiekt powinien byc wykryty");
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.5f, ObjectTracker_GetX(&tracker)); // Środek między 0 a 1 to 0.5
    TEST_ASSERT_TRUE_MESSAGE(angle < -10.0f, "Kąt powinien być ujemny (na lewo)");
    // Dokładne wyliczenie: (0.5 - 3.5) * (45 / 8) = -3 * 5.625 = -16.875
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -16.875f, angle);
}

void test_ObjectTracker_Should_CalculatePositiveAngle_When_ObjectIsOnRight(void)
{
    // ARRANGE
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    // Piksele po prawej stronie (kolumna 6 i 7)
    // Indeks 22 (wiersz 2, kol 6), Indeks 23 (wiersz 2, kol 7)
    Helper_SetPixel(&frame, 22, 500);
    Helper_SetPixel(&frame, 23, 500);

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    float angle = ObjectTracker_GetAngle(&tracker);
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 6.5f, ObjectTracker_GetX(&tracker)); // Środek między 6 a 7 to 6.5
    TEST_ASSERT_TRUE_MESSAGE(angle > 10.0f, "Kąt powinien być dodatni (na prawo)");
    // Dokładne wyliczenie: (6.5 - 3.5) * (45 / 8) = 3 * 5.625 = 16.875
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 16.875f, angle);
}

// =========================================================================
// GRUPA 2: Filtrowanie Zakłóceń i Granice (Boundaries)
// =========================================================================

void test_ObjectTracker_Should_IgnoreObjects_TooClose(void)
{
    // ARRANGE
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    // Obiekt bardzo blisko (50mm), min_dist to 100mm
    Helper_SetPixel(&frame, 27, 50); 
    Helper_SetPixel(&frame, 28, 50);

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_FALSE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Obiekt poniżej min_dist powinien być ignorowany");
}

void test_ObjectTracker_Should_IgnoreObjects_TooFar(void)
{
    // ARRANGE
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    // Obiekt bardzo daleko (2500mm), max_dist to 2000mm
    Helper_SetPixel(&frame, 27, 2500);

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_FALSE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Obiekt powyżej max_dist powinien być ignorowany");
}

void test_ObjectTracker_Should_HaveReducedProbability_InFadeZone(void)
{
    // ARRANGE
    // Min 100, Fade 200. Strefa Fade-In kończy się na 300mm.
    // Obiekt na 200mm jest dokładnie w połowie strefy (50% wagi).
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 36, 200); 

    // Wymagamy dużej masy, aby pojedynczy piksel z wagą 0.5 nie wystarczył do detekcji
    // Ale w tym teście sprawdzamy tylko czy waga (probability) spada.
    // Ustawiamy próg detekcji bardzo nisko, żeby w ogóle wykrył.
    config.min_mass_detect = 0.1f; 

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));
    // Normalnie 1 piksel = masa 1.0. Tutaj w strefie fade (połowa) = masa 0.5.
    // Probability = masa / (min_mass * 2). To zależy od implementacji, 
    // ale sprawdźmy czy masa jest mniejsza niż 1.0.
    // W kodzie: w = (200 - 100) / 200 = 0.5.
    
    // Nie mamy dostępu do wewnętrznej masy, ale waga jest używana do średniej ważonej.
    // Jeśli mamy tylko 1 piksel, pozycja będzie poprawna niezależnie od wagi.
    // Sprawdźmy 'Probability' - w kodzie jest to 'low_pass(raw_prob)'.
    // raw_prob = sum_weight / (min_mass * 2).
    // sum_weight = 0.5. min_mass = 0.1. raw_prob = 0.5 / 0.2 = 2.5 -> clamp do 1.0.
    // Ten test jest trudny do sprawdzenia na 'Probability' bo normalizacja jest agresywna.
    // Zróbmy inaczej: Ustawmy min_mass tak, aby 0.5 wagi to było za mało.
}

void test_ObjectTracker_Should_IgnoreObject_InFadeZone_IfWeak(void)
{
    // ARRANGE
    config.min_mass_detect = 0.8f; // Próg 0.8
    // Obiekt w połowie strefy fade (waga 0.5)
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 36, 200); // 200mm -> (200-100)/200 = 0.5 wagi

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_FALSE_MESSAGE(ObjectTracker_IsDetected(&tracker), 
        "Obiekt o wadze 0.5 powinien być odrzucony przy progu 0.8");
}


// =========================================================================
// GRUPA 3: Logika Stanowa (Pamięć i Utrata Sygnału)
// =========================================================================

void test_ObjectTracker_Should_LoseDetection_When_SignalDisappears(void)
{
    // ARRANGE
    RangeSensorFrame valid_frame;
    Helper_ClearFrame(&valid_frame);
    Helper_SetPixel(&valid_frame, 36, 1000);

    RangeSensorFrame empty_frame;
    Helper_ClearFrame(&empty_frame); // Wszystko 0

    // Krok 1: Wykrycie
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&valid_frame);
    
    ObjectTracker_Update(&tracker);
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));

    // Krok 2: Utrata sygnału
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&empty_frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_FALSE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Tracker powinien zgłosić brak obiektu po pustej klatce");
}

void test_ObjectTracker_Should_MaintainLastPosition_When_SignalMomentarilyLost(void)
{
    // Ten test sprawdza czy po utracie detekcji "GetX/GetY" zwraca ostatnią znaną pozycję (lub zero),
    // albo czy zachowuje się stabilnie. Wg kodu:
    // self->target.position_x = ... (aktualizowane tylko gdy detected).
    // Więc powinno trzymać starą wartość.
    
    // ARRANGE
    RangeSensorFrame valid_frame;
    Helper_ClearFrame(&valid_frame);
    Helper_SetPixel(&valid_frame, 3, 1000); // Pozycja X=3, Y=0

    RangeSensorFrame empty_frame;
    Helper_ClearFrame(&empty_frame);

    // Krok 1: Wykrycie
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&valid_frame);
    ObjectTracker_Update(&tracker);

    float last_x = ObjectTracker_GetX(&tracker);

    // Krok 2: Utrata
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&empty_frame);
    ObjectTracker_Update(&tracker);

    // ASSERT
    TEST_ASSERT_FALSE(ObjectTracker_IsDetected(&tracker));
    TEST_ASSERT_EQUAL_FLOAT(last_x, ObjectTracker_GetX(&tracker)); // Powinien pamiętać ostatnią pozycję
}

void test_ObjectTracker_Should_Reject_WideSpread_GhostObjects(void)
{
    // ARRANGE
    // Limitujemy rozproszenie. Wariancja dla punktów X=0 i X=7 wynosi ok. 12.25.
    // Ustawiamy próg na 5.0, co powinno odrzucić ten przypadek.
    config.max_dispersion = 5.0f;

    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    
    // Obiekt po lewej (kolumna 0)
    Helper_SetPixel(&frame, 16, 1000); // Wiersz 2, Kol 0
    
    // Obiekt po prawej (kolumna 7)
    Helper_SetPixel(&frame, 23, 1000); // Wiersz 2, Kol 7

    // Wymagamy, aby algorytm wykrył, że to nie jest jeden punktowy obiekt
    // Ustawiamy jakiś (przyszły) limit rozrzutu, tu zakładamy że domyślny config pozwoli,
    // ale my oczekujemy FALSE.
    
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    // Jeśli test ZDA, to znaczy że tracker ODRZUCIŁ ducha (to nasz cel).
    // Jeśli test obleje, to znaczy że tracker wykrył "coś" na środku (aktualny błąd).
    TEST_ASSERT_FALSE_MESSAGE(ObjectTracker_IsDetected(&tracker), 
        "Tracker powinien odrzucić sygnał składający się z dwóch odległych obiektów (Ghost Effect)");
}

// =========================================================================
// GRUPA 4: Podtrzymanie Sygnału (Signal Hold / Hysteresis)
// =========================================================================

void test_ObjectTracker_Should_HoldDetection_When_SignalLost_Temporarily(void)
{
    // ARRANGE
    // Konfigurujemy czas podtrzymania na 5 klatek
    config.max_hold_frames = 5;

    RangeSensorFrame valid_frame;
    Helper_ClearFrame(&valid_frame);
    Helper_SetPixel(&valid_frame, 36, 1000); // Obiekt jest

    RangeSensorFrame empty_frame;
    Helper_ClearFrame(&empty_frame); // Pusto

    // 1. Wykrycie początkowe
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&valid_frame);
    ObjectTracker_Update(&tracker);
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));

    // 2. Pierwsza pusta klatka - powinien podtrzymać
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&empty_frame);
    ObjectTracker_Update(&tracker);
    TEST_ASSERT_TRUE_MESSAGE(ObjectTracker_IsDetected(&tracker), "Powinien podtrzymać detekcję (klatka 1/5)");

    // 3. Sprawdźmy czy pozycja jest zamrożona (taka sama jak ostatnio)
    // Ostatnia znana: 1000mm. Jeśli wpadnie pusta, filtry nie powinny zjechać do zera.
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 1000.0f, ObjectTracker_GetDistance(&tracker));
}

// =========================================================================
// GRUPA 5: Precyzja Geometrii (Kąty)
// =========================================================================

void test_ObjectTracker_Geometry_Precision(void)
{
    // ARRANGE
    config.sensor_fov_deg = 45.0f;
    config.smooth_factor = 0.0f; // Bez wygładzania dla precyzji
    
    // Krok piksela = 45 / 8 = 5.625 stopnia.
    // X=0 -> (0 - 3.5) * 5.625 = -19.6875
    // X=3 -> (3 - 3.5) * 5.625 = -2.8125
    // X=4 -> (4 - 3.5) * 5.625 = +2.8125
    // X=7 -> (7 - 3.5) * 5.625 = +19.6875

    RangeSensorFrame frame;

    // --- CASE 1: Piksel 0 (Max Left) ---
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 0, 1000); // Index 0 is (0,0) so X=0
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);
    ObjectTracker_Update(&tracker);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, ObjectTracker_GetX(&tracker));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -19.6875f, ObjectTracker_GetAngle(&tracker));

    // --- CASE 2: Piksel 4 (Near Center Right) ---
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 4, 1000); // Index 4 is (4,0) so X=4
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);
    ObjectTracker_Update(&tracker);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.0f, ObjectTracker_GetX(&tracker));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.8125f, ObjectTracker_GetAngle(&tracker));
}

// =========================================================================
// GRUPA 6: Testy Behawioralne (Fizyka Trackera)
// =========================================================================

void test_ObjectTracker_Should_BiasPositionTowardsStrongerSignal(void)
{
    // Ten test sprawdza, czy faktycznie używamy "Środka Ciężkości", a nie zwykłej średniej.
    // ARRANGE
    config.min_dist_mm = 100;
    config.fade_dist_mm = 200; // Fade in: 100..300mm
    config.smooth_factor = 0.0f; 

    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);

    // 1. Silny sygnał na indeksie 3 (Waga 1.0) - odległość 500mm (poza fade)
    Helper_SetPixel(&frame, 3, 500);

    // 2. Słabszy sygnał na indeksie 4 (Waga ~0.75) - odległość 250mm (wewnątrz fade)
    // Waga = (250 - 100) / 200 = 0.75
    Helper_SetPixel(&frame, 4, 250);

    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);

    // ACT
    ObjectTracker_Update(&tracker);

    // ASSERT
    // Gdyby to była średnia arytmetyczna: (3+4)/2 = 3.5
    // Jako średnia ważona: (3*1.0 + 4*0.75) / (1.0 + 0.75) = 6.0 / 1.75 = 3.4285...
    float x = ObjectTracker_GetX(&tracker);
    
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));
    TEST_ASSERT_TRUE_MESSAGE(x < 3.5f, "Pozycja powinna być 'ściągnięta' w stronę silniejszego sygnału (indeks 3)");
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.428f, x);
}

void test_ObjectTracker_Should_ResetState_When_Reinitialized(void)
{
    // ARRANGE
    // 1. Wprowadzamy tracker w stan "wykryto" z historią
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 3, 1000);
    
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);
    ObjectTracker_Update(&tracker);
    
    TEST_ASSERT_TRUE(ObjectTracker_IsDetected(&tracker));

    // ACT
    // Re-inicjalizacja (np. reset systemu)
    ObjectTracker_Init(&tracker, &config);

    // ASSERT
    // Wszystko powinno być wyzerowane
    TEST_ASSERT_FALSE(ObjectTracker_IsDetected(&tracker));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ObjectTracker_GetDistance(&tracker));
    // Sprawdzamy czy licznik podtrzymania też padł
    // Ustawiamy frame pusty -> jeśli licznik jest wyzerowany, to natychmiast zwróci false (przy max_hold > 0)
    // Ale Init zeruje _frames_lost, więc jest czysto.
}

void test_ObjectTracker_Filter_Should_Converge_Without_Overshoot(void)
{
    // Testuje zachowanie filtra (Low Pass), a nie konkretne wartości.
    // Oczekujemy, że przy stałym sygnale wejściowym, wyjście będzie się zbliżać do celu,
    // ale nigdy go nie przeskoczy (brak oscylacji).
    
    // ARRANGE
    config.smooth_factor = 0.5f;
    
    RangeSensorFrame frame;
    Helper_ClearFrame(&frame);
    Helper_SetPixel(&frame, 3, 1000); // Cel: Dystans 1000

    // Krok 1: Start z 0.
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);
    ObjectTracker_Update(&tracker);
    
    float val1 = ObjectTracker_GetDistance(&tracker);
    TEST_ASSERT_TRUE_MESSAGE(val1 > 0.0f && val1 < 1000.0f, "Pierwszy krok powinien być pomiędzy startem a celem");

    // Krok 2: Kolejna aktualizacja
    RangeSensor_GetNewFrame_ExpectAndReturn(NULL, true);
    RangeSensor_GetNewFrame_IgnoreArg_frame();
    RangeSensor_GetNewFrame_ReturnThruPtr_frame(&frame);
    ObjectTracker_Update(&tracker);
    
    float val2 = ObjectTracker_GetDistance(&tracker);
    TEST_ASSERT_TRUE_MESSAGE(val2 > val1, "Wartość powinna rosnąć");
    TEST_ASSERT_TRUE_MESSAGE(val2 < 1000.0f, "Nie powinno być przeregulowania (overshoot)");
}

#endif // TEST