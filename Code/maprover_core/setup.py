from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'maprover_core'

setup(
    name='maprover_core',
    version='0.0.0',
    packages=['maprover_core'],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/maprover_core']),
        ('share/' + 'maprover_core', ['package.xml']),
        # Tell ROS 2 to include our new launch and config files
        (os.path.join('share', 'maprover_core', 'launch'), glob('launch/*.launch.py')),
        (os.path.join('share', 'maprover_core', 'config'), glob('config/*.yaml')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='sreshtlinux',
    maintainer_email='sreshtlinux@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
        ],
    },
)
