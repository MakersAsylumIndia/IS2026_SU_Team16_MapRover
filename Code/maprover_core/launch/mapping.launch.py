from launch import LaunchDescription
from launch_ros.actions import LifecycleNode, Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    ydlidar_params = os.path.join(
        get_package_share_directory('ydlidar_ros2_driver'),
        'params', 'ydlidar.yaml')

    slam_config = os.path.join(
        get_package_share_directory('maprover_core'),
        'config', 'mapper_params_online_async.yaml')

    ekf_config = os.path.join(
        get_package_share_directory('maprover_core'),
        'config', 'ekf.yaml')

    static_tf_laser = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_laser',
        arguments=[
            '--x', '0', '--y', '0', '--z', '0.02',
            '--roll', '0', '--pitch', '0', '--yaw', '0',
            '--frame-id', 'base_link',
            '--child-frame-id', 'laser_frame'
        ]
    )

    static_tf_imu = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_imu',
        arguments=[
            '--x', '0', '--y', '0', '--z', '0',
            '--roll', '0', '--pitch', '0', '--yaw', '0',
            '--frame-id', 'base_link',
            '--child-frame-id', 'imu_link'
        ]
    )

    imu_node = Node(
        package='maprover_core',
        executable='imu_publisher',
        name='imu_publisher',
        output='screen',
    )

    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config],
    )

    lidar_node = LifecycleNode(
        package='ydlidar_ros2_driver',
        executable='ydlidar_ros2_driver_node',
        name='ydlidar_ros2_driver_node',
        output='screen',
        emulate_tty=True,
        parameters=[ydlidar_params],
        namespace='/',
    )

    slam_node = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[slam_config],
    )

    return LaunchDescription([
        static_tf_laser,
        static_tf_imu,
        imu_node,
        ekf_node,
        lidar_node,
        slam_node,
    ])
