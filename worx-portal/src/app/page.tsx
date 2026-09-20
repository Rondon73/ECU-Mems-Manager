"use client";

import { useMemo, useState } from "react";
import { DEFAULT_BOTS, MODEL_OPTIONS, type BotConfig } from "@/lib/bots";

type ConnectionState = {
  status: "idle" | "success" | "error";
  message: string;
};

const idleConnection: ConnectionState = {
  status: "idle",
  message: "Not connected",
};

export default function Home() {
  const [bots, setBots] = useState<BotConfig[]>(DEFAULT_BOTS);
  const [notionToken, setNotionToken] = useState("");
  const [notionDatabaseId, setNotionDatabaseId] = useState("");
  const [chatGptKey, setChatGptKey] = useState("");
  const [chatGptModel, setChatGptModel] = useState("gpt-4.1-mini");
  const [notionConnection, setNotionConnection] =
    useState<ConnectionState>(idleConnection);
  const [chatGptConnection, setChatGptConnection] =
    useState<ConnectionState>(idleConnection);
  const [busy, setBusy] = useState<"notion" | "chatgpt" | null>(null);

  const connectedCount = useMemo(() => {
    const states = [notionConnection, chatGptConnection];
    return states.filter((state) => state.status === "success").length;
  }, [chatGptConnection, notionConnection]);

  const updateBotModel = (id: string, model: string) => {
    setBots((current) =>
      current.map((bot) => (bot.id === id ? { ...bot, model } : bot)),
    );
  };

  const connectNotion = async () => {
    setBusy("notion");
    setNotionConnection({ status: "idle", message: "Connecting..." });

    try {
      const response = await fetch("/api/connect/notion", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          token: notionToken,
          databaseId: notionDatabaseId,
        }),
      });

      const data = (await response.json()) as { ok: boolean; message: string };
      setNotionConnection({
        status: data.ok ? "success" : "error",
        message: data.message,
      });
    } catch {
      setNotionConnection({
        status: "error",
        message: "Unable to reach Notion connector.",
      });
    } finally {
      setBusy(null);
    }
  };

  const connectChatGpt = async () => {
    setBusy("chatgpt");
    setChatGptConnection({ status: "idle", message: "Connecting..." });

    try {
      const response = await fetch("/api/connect/chatgpt", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ apiKey: chatGptKey, model: chatGptModel }),
      });

      const data = (await response.json()) as { ok: boolean; message: string };
      setChatGptConnection({
        status: data.ok ? "success" : "error",
        message: data.message,
      });
    } catch {
      setChatGptConnection({
        status: "error",
        message: "Unable to reach ChatGPT connector.",
      });
    } finally {
      setBusy(null);
    }
  };

  return (
    <div className="mx-auto w-full max-w-6xl px-6 py-10">
      <header className="mb-8">
        <p className="text-sm font-semibold uppercase tracking-widest text-blue-600">
          Worx Equipe
        </p>
        <h1 className="mt-2 text-3xl font-bold">Frontend Portal</h1>
        <p className="mt-3 text-sm text-zinc-600 dark:text-zinc-300">
          Manage bots, assign models, and verify integrations with Notion and
          ChatGPT from one dashboard.
        </p>
      </header>

      <section className="mb-8 grid gap-4 md:grid-cols-3">
        <Card title="Team bots" value={`${bots.length}`} />
        <Card title="Connected services" value={`${connectedCount}/2`} />
        <Card title="Primary stack" value="Notion + ChatGPT" />
      </section>

      <section className="mb-8 grid gap-6 lg:grid-cols-2">
        <IntegrationCard
          title="Notion"
          status={notionConnection}
          actionLabel="Connect Notion"
          busy={busy === "notion"}
          onSubmit={connectNotion}
        >
          <Input
            label="Internal Integration Token"
            type="password"
            value={notionToken}
            onChange={setNotionToken}
            placeholder="secret_..."
          />
          <Input
            label="Database ID (optional)"
            value={notionDatabaseId}
            onChange={setNotionDatabaseId}
            placeholder="XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
          />
        </IntegrationCard>

        <IntegrationCard
          title="ChatGPT"
          status={chatGptConnection}
          actionLabel="Connect ChatGPT"
          busy={busy === "chatgpt"}
          onSubmit={connectChatGpt}
        >
          <Input
            label="OpenAI API Key"
            type="password"
            value={chatGptKey}
            onChange={setChatGptKey}
            placeholder="sk-..."
          />
          <label className="flex flex-col gap-2 text-sm font-medium">
            Model
            <select
              value={chatGptModel}
              onChange={(event) => setChatGptModel(event.target.value)}
              className="rounded-md border border-zinc-300 px-3 py-2 text-sm outline-none focus:border-blue-500"
            >
              {MODEL_OPTIONS.filter((model) => model.startsWith("gpt")).map(
                (model) => (
                  <option key={model} value={model}>
                    {model}
                  </option>
                ),
              )}
            </select>
          </label>
        </IntegrationCard>
      </section>

      <section className="rounded-xl border border-zinc-200 bg-white p-5 shadow-sm dark:border-zinc-800 dark:bg-zinc-950">
        <h2 className="text-lg font-semibold">Bot model assignment</h2>
        <p className="mt-1 text-sm text-zinc-600 dark:text-zinc-300">
          Choose the best model per bot role.
        </p>
        <div className="mt-5 grid gap-3 md:grid-cols-2">
          {bots.map((bot) => (
            <div
              key={bot.id}
              className="rounded-lg border border-zinc-200 p-4 dark:border-zinc-800"
            >
              <div className="mb-3 flex items-center gap-3">
                <span className="text-2xl" aria-hidden>
                  {bot.avatar}
                </span>
                <div>
                  <h3 className="font-semibold">{bot.name}</h3>
                  <p className="text-sm text-zinc-600 dark:text-zinc-300">
                    {bot.role}
                  </p>
                </div>
              </div>

              <label className="flex flex-col gap-2 text-sm font-medium">
                Assigned model
                <select
                  value={bot.model}
                  onChange={(event) =>
                    updateBotModel(bot.id, event.target.value)
                  }
                  className="rounded-md border border-zinc-300 px-3 py-2 text-sm outline-none focus:border-blue-500"
                >
                  {MODEL_OPTIONS.map((model) => (
                    <option key={model} value={model}>
                      {model}
                    </option>
                  ))}
                </select>
              </label>
            </div>
          ))}
        </div>
      </section>
    </div>
  );
}

function Card({ title, value }: { title: string; value: string }) {
  return (
    <article className="rounded-xl border border-zinc-200 bg-white p-5 shadow-sm dark:border-zinc-800 dark:bg-zinc-950">
      <p className="text-sm text-zinc-500 dark:text-zinc-400">{title}</p>
      <p className="mt-2 text-2xl font-semibold">{value}</p>
    </article>
  );
}

function IntegrationCard({
  title,
  status,
  actionLabel,
  busy,
  onSubmit,
  children,
}: {
  title: string;
  status: ConnectionState;
  actionLabel: string;
  busy: boolean;
  onSubmit: () => Promise<void>;
  children: React.ReactNode;
}) {
  return (
    <article className="rounded-xl border border-zinc-200 bg-white p-5 shadow-sm dark:border-zinc-800 dark:bg-zinc-950">
      <h2 className="text-lg font-semibold">{title}</h2>
      <p
        className={`mt-2 text-sm ${
          status.status === "success"
            ? "text-green-600"
            : status.status === "error"
              ? "text-red-600"
              : "text-zinc-600 dark:text-zinc-300"
        }`}
      >
        {status.message}
      </p>

      <div className="mt-4 space-y-3">{children}</div>

      <button
        type="button"
        onClick={onSubmit}
        disabled={busy}
        className="mt-4 inline-flex rounded-md bg-blue-600 px-4 py-2 text-sm font-medium text-white hover:bg-blue-700 disabled:cursor-not-allowed disabled:opacity-60"
      >
        {busy ? "Connecting..." : actionLabel}
      </button>
    </article>
  );
}

function Input({
  label,
  value,
  onChange,
  placeholder,
  type = "text",
}: {
  label: string;
  value: string;
  onChange: (next: string) => void;
  placeholder: string;
  type?: "text" | "password";
}) {
  return (
    <label className="flex flex-col gap-2 text-sm font-medium">
      {label}
      <input
        type={type}
        value={value}
        onChange={(event) => onChange(event.target.value)}
        placeholder={placeholder}
        className="rounded-md border border-zinc-300 px-3 py-2 text-sm outline-none focus:border-blue-500"
      />
    </label>
  );
}
