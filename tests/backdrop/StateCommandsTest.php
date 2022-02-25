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
    $output_all = shell_exec('bee state-get maintenance_mode');
    $this->assertStringContainsString('0', $output_all);
    $this->assertStringContainsString('maintenance_mode', $output_all);
  }

  /**
   * Make sure that the state-set command works.
   */
  public function test_state_set_command_works() {
    $output = shell_exec('bee state-get maintenance_mode');
    $this->assertStringContainsString('maintenance_mode', $output);
    $this->assertStringContainsString('0', $output);

    $output = shell_exec('bee state-set maintenance_mode 1');
    $this->assertStringContainsString('1', $output);
  }

    /**
   * Make sure that the maintenance-mode command works.
   */
  public function test_maintenance_mode_command_works() {
    $output = shell_exec('bee maintenance_mode 1');
    $this->assertStringContainsString('maintenance_mode', $output);
    $this->assertStringContainsString('1', $output);

    $output = shell_exec('bee state-set maintenance_mode 0');
    $this->assertStringContainsString('0', $output);
  }
}
