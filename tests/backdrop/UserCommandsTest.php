<?php
/**
 * @file
 * PHPUnit tests for Bee User commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for managing users.
 */
class UserCommandsTest extends TestCase {

  /**
   * Make sure that the users command works.
   */
  public function test_users_command_works() {
    $output = shell_exec('bee users');
    $this->assertRegExp('/| 1 +| admin +| admin@example.com +|/', (string) $output);
  }

  /**
   * Make sure that the user-password command works.
   */
  public function test_user_password_command_works() {
    $output_password = shell_exec('bee user-password admin 123456');
    $this->assertStringContainsString("The password for 'admin' has been reset.", (string) $output_password);
    $this->assertStringNotContainsString('The new password is:', (string) $output_password);

    $output_random = shell_exec('bee user-password admin');
    $this->assertStringContainsString("The password for 'admin' has been reset.", (string) $output_random);
    $this->assertStringContainsString('The new password is:', (string) $output_random);
  }

  /**
   * Make sure that the user-login command works.
   */
  public function test_user_login_command_works() {
    $output = shell_exec('bee user-login admin');
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output);
    $this->assertStringContainsString('/user/reset/1/', (string) $output);

    // Test that leaving the username argument blank loads User 1.
    $output = shell_exec('bee user-login');
    $this->assertStringContainsString("Use the following link to login as 'admin':", (string) $output);
    $this->assertStringContainsString('/user/reset/1/', (string) $output);
  }

}
