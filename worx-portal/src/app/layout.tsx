import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Worx Equipe Portal",
  description: "Manage AI team bots and service integrations.",
};

export default function RootLayout({ children }: LayoutProps<"/">) {
  return (
    <html lang="en" className="h-full antialiased">
      <body className="min-h-full flex flex-col bg-zinc-50 text-zinc-950 dark:bg-zinc-900 dark:text-zinc-50">
        {children}
      </body>
    </html>
  );
}
