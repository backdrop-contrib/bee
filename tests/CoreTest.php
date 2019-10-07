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
    $output_b = shell_exec('b');
    $output_help = shell_exec('b help');

    $this->assertEquals($output_help, $output_b);
  }

  /**
   * Make sure that unknown commands display an error.
   */
  public function testUnknownCommandDisplaysError() {
    // Will have to change this if we ever add a 'spoon' command ;-)
    $command = 'spoon';
    $output = shell_exec("b $command");

    $this->assertStringContainsString("There is no $command", $output);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function testRequiredArgumentsAreRequired() {
    $output = shell_exec('b config-get');

    $this->assertStringContainsString('Argument config_name is required', $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function testCommandAliasesWork() {
    $output_alias = shell_exec('b st');
    $output_command = shell_exec('b core-status');

    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--root` option works.
   */
  public function testRootOptionWorks() {
    print_r(getcwd());
    exec('cd ../');
    print_r(getcwd());
    print_r($output_no_root = shell_exec('b st'));
    print_r($output_root = shell_exec('b --root=backdrop st'));

    $this->assertEquals($output_no_root, $output_root);
  }

  /**
   * Make sure that the `--y/--yes` options work.
   */
  public function testYesOptionWorks() {
    $output_y = shell_exec('b en book --y');
    $output_yes = shell_exec('b dis book --yes');

    $this->assertStringContainsString('Module book enabled', $output_y);
    $this->assertStringContainsString('Module book disabled', $output_yes);
  }

}

