Ralio Products
===========================================

# Mercury v2.0 Microcontroller Board Testing

## Power Circuit Test Results

### 1. Bootup

- Smooth bootup with Vcc voltage ranging between 6V to 15V

![boot_6V](img/boot_6V.png) ![boot_9V](img/boot_9V.png) ![boot_12V](img/boot_12V.png) ![boot_15V](img/boot_15v.png) 

___

### 2. Motor Stall Characteristics with 9V Alkaline Battery

#### 2.1 New Battery - Fully Charged (Vbat > 9V)

- Vcc ~ 8.5V
- Two motors connected to channel 1 and 2 (one each).
- Both motors made to stall simultaneously when in-drive. Stall current ~ 750mA per motor
- 3V rail (blue trace) remains stable

![dualMotorStall_9V_VBAT](img/dualMotorStall_9V_VBAT.png) 

#### 2.2 Used Battery (Vbat ~ 8V)

- Vcc ~ 7.5V
- Two motors connected to channel 1 and 2 (one each).
- Both motors made to stall simultaneously when in-drive. Stall current ~750 mA per motor
> [!CAUTION]
> 3V rail (blue trace) drops to 1.7V

![dualMotorStall_8V_VBAT](img/dualMotorStall_8V_VBAT.png) 

___

### 3. Motor Driver Performance

#### 3.1 Dual Channel Nominal Operation

- Two motors (eloads) connected to channel 1 and 2 (one each).
- Both motors driven for 15min consuming 350mA per channel (700mA) total.
> [!NOTE]
> - Motor driver successfully supported 700mA current consumption for 15min.
> - Motor driver remains below 35C throughout the test.

![dualChNom](img/dualChNom.png)

#### 3.2 Dual Channel Stall Operation

- Two motors (eloads) connected to channel 1 and 2 (one each).
- Both motors driven for 15min consuming 700mA per channel (1.4A) total
> [!NOTE]
> - Motor driver successfully supported 1.4A current consumption for 15min.
> - Motor driver remains below 40C throughout the test.

![dualChStall](img/dualChStall.png)

#### 3.3 Dual Channel Maximum Current Capability

- Two motors (eloads) connected to channel 1 and 2 (one each).
- Both motors driven for 15min consuming 1A per channel (2A) total
> [!NOTE]
> - Motor driver successfully supported 2A current consumption for 15min.

> [!WARNING]
> - Motor driver remains below 55C throughout the test.

> [!TIP]
> **It's recommended to not consume more that 1A per channel for more than 5min.**

![dualChStall](img/dualChStall.png)

#### Test Setup

![dualMotorTestSetup](img/dualMotorTestSetup.png)

___

### 4. Servo Motor Driver Performance and effects on 5V rail

- Servo sweep test conducted with 100g, 200g, 300g and 400g load
- Load arm length ~ 13.5mm
> [!TIP]
**It's recommended to not exceed 0.025Nm torge per servo.**

#### 4.1 Servo Sweep Operation with 100g load

> [!NOTE]
> - Servo can successfully sweep from 0 - 180degree providing 0.013Nm torque
- Average current consumption < 20mA with spikes of 500mA

![servo_100g](img/servo_100g.png)

#### 4.2 Servo Sweep Operation with 200g load

> [!NOTE]
> - Servo can successfully sweep from 0 - 180degree providing 0.013Nm torque.
- Average current consumption < 40mA with spikes of 500mA

![servo_200g](img/servo_200g.png)

#### 4.3 Servo Sweep Operation with 300g load

> [!WARNING]
> Servo can successfully sweep from 0 - 180degree providing 0.040Nm torque but as soon as power is released, the servo motor is not able to hold the 300g load and it drops down.

- Average current consumption < 60mA with spikes of 500mA

![servo_300g](img/servo_300g.png)

#### 4.4 Servo Sweep Operation with 400g load

> [!CAUTION]
Servo failed to sweep and stalled as soon as test began causing permanent damage.
- Stall current < 900mA

![servo_400g](img/servo_400g.png)

#### Test Setup

![servoTestSetup](img/servoTestSetup.png)

___
