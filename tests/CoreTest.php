<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console functionality.
 */

use PHPUnit\Framework\TestCase;

class CoreTest extends TestCase {

  /**
   * Make sure that `b` is installed correctly.
   */
  public function testBInstalledCorrectly() {
    exec('b', $output, $result);
    $this->assertEquals(0, $result);
  }

  /**
   * Make sure that `help` is used as the the default command (when no command
   * is given).
   */
  public function testHelpIsDefaultCommand() {
    exec('b', $output_b);
    exec('b help', $output_help);
    $this->assertEquals($output_help, $output_b);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function testRequiredArgumentsAreRequired() {
    exec('b config-get', $output);
    $this->assertContains('Argument config_name is required', $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function testCommandAliasesWork() {
    exec('b st', $output_alias);
    exec('b core-status', $output_command);
    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--y/--yes` options work.
   */
  public function testYesOptionWorks() {
    exec('b en book --y', $output);
    $this->assertContains('Module book enabled', $output);
  }

}

