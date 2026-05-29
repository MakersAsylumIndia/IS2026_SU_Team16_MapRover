import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
from mpu6050 import mpu6050
import math

class ImuPublisher(Node):
    def __init__(self):
        super().__init__('imu_publisher')

        # MPU-6050 on I2C address 0x68
        self.sensor = mpu6050(0x68)

        self.pub = self.create_publisher(Imu, '/imu/data', 10)

        # Publish at 50 Hz — fast enough for rotation tracking
        self.timer = self.create_timer(0.02, self.publish_imu)
        self.get_logger().info('IMU publisher started on /imu/data')

    def publish_imu(self):
        try:
            accel = self.sensor.get_accel_data()  # m/s^2
            gyro = self.sensor.get_gyro_data()     # deg/s
        except Exception as e:
            self.get_logger().warn(f'IMU read failed: {e}')
            return

        msg = Imu()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'imu_link'

        # Linear acceleration (already in m/s^2 from this library)
        msg.linear_acceleration.x = accel['x']
        msg.linear_acceleration.y = accel['y']
        msg.linear_acceleration.z = accel['z']

        # Angular velocity — convert deg/s to rad/s (this is what fixes rotation)
        msg.angular_velocity.x = math.radians(gyro['x'])
        msg.angular_velocity.y = math.radians(gyro['y'])
        msg.angular_velocity.z = math.radians(gyro['z'])

        # We don't compute absolute orientation here; mark it unknown
        # (orientation_covariance[0] = -1 tells consumers to ignore orientation)
        msg.orientation_covariance[0] = -1.0

        # Covariances: rough values telling the EKF how much to trust each reading
        msg.angular_velocity_covariance[0] = 0.01
        msg.angular_velocity_covariance[4] = 0.01
        msg.angular_velocity_covariance[8] = 0.01
        msg.linear_acceleration_covariance[0] = 0.1
        msg.linear_acceleration_covariance[4] = 0.1
        msg.linear_acceleration_covariance[8] = 0.1

        self.pub.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = ImuPublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
