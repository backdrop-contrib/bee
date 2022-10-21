<?php
/**
 * @file
 * PHPUnit tests for Bee State commands.
 */

use PHPUnit\Framework\TestCase;

class StateCommandsTest extends TestCase {

  /**
   * Make sure that the state-get command works.
   */
  public function test_state_get_command_works() {
    // New installs don't have maintenance_mode state so need setting
    $output = shell_exec('bee maintenance_mode FALSE');
    
    $output = shell_exec('bee state-get maintenance_mode');
    $this->assertStringContainsString("The value of the 'maintenance_mode' state is: FALSE", (string) $output);
  }

  /**
   * Make sure that the state-set command works.
   */
  public function test_state_set_command_works() {
    // Make sure maintenance_mode is '0'.
    $output = shell_exec('bee state-get maintenance_mode');
    $this->assertStringContainsString("The value of the 'maintenance_mode' state is: FALSE", (string) $output);
    
    // Set maintenance_mode to '1'.
    $output = shell_exec('bee state-set maintenance_mode TRUE');
    $this->assertStringContainsString("The 'maintenance_mode' state was set to: TRUE", (string) $output);
    
    // Make sure maintenance_mode is '1'.
    $output = shell_exec('bee state-get maintenance_mode');
    $this->assertStringContainsString("The value of the 'maintenance_mode' state is: TRUE", (string) $output);
  }

  /**
   * Make sure that the maintenance-mode command works.
   */
  public function test_maintenance_mode_command_works() {
    // Attempt to enable maintenance mode
    $output = shell_exec('bee maintenance-mode TRUE');
    $this->assertStringContainsString('Maintenance mode is already enabled', (string) $output);

    // Disable maintenance mode
    $output = shell_exec('bee maintenance-mode FALSE');
    $this->assertStringContainsString('Maintenance mode was disabled', (string) $output);

    // Enable maintenance mode
    $output = shell_exec('bee maintenance-mode TRUE');
    $this->assertStringContainsString('Maintenance mode was enabled', (string) $output);

    // Get the status of maintenance mode
    $output = shell_exec('bee maintenance-mode');
    $this->assertStringContainsString('Maintenance mode is enabled', (string) $output);

    // Disable maintenance mode
    $output = shell_exec('bee maintenance-mode FALSE');
    $this->assertStringContainsString('Maintenance mode was disabled', (string) $output);

    // Get the status of maintenance mode
    $output = shell_exec('bee maintenance-mode');
    $this->assertStringContainsString('Maintenance mode is disabled', (string) $output);
  }
}
