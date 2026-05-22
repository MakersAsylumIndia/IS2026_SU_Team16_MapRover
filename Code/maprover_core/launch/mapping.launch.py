from launch import LaunchDescription
from launch_ros.actions import LifecycleNode, Node
from launch.actions import TimerAction
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    ydlidar_params = os.path.join(
        get_package_share_directory('ydlidar_ros2_driver'),
        'params',
        'ydlidar.yaml'
    )

    slam_config = os.path.join(
        get_package_share_directory('maprover_core'),
        'config',
        'mapper_params_online_async.yaml'
    )

    static_tf_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_odom',
        arguments=['0', '0', '0', '0', '0', '0', '1', 'odom', 'base_link']
    )

    static_tf_laser = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_node',
        arguments=['0', '0', '0.02', '0', '0', '0', '1', 'base_link', 'laser_frame']
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

    slam_node = TimerAction(
        period=5.0,
        actions=[
            Node(
                package='slam_toolbox',
                executable='async_slam_toolbox_node',
                name='slam_toolbox',
                output='screen',
                parameters=[slam_config],
            )
        ]
    )

    return LaunchDescription([
        static_tf_odom,
        static_tf_laser,
        lidar_node,
        slam_node,    # No relay node
    ])

